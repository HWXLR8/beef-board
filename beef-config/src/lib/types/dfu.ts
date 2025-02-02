// Thanks to dfu-programmer for being the reference implementation

import { EventEmitter } from "eventemitter3";
import { Buffer } from "buffer";
import type MemoryMap from "nrf-intel-hex";

// DFU Constants from Datasheet
const DFU_VID_AT90USB1286 = 0x03eb;
const DFU_PID_AT90USB1286 = 0x2ffb;
const DFU_INTERFACE = 0;

const ATMEL_CONTROL_BLOCK_SIZE = 32;
export const ATMEL_MAX_TRANSFER_SIZE = 0x400;

// DFU Class-Specific Requests
enum DfuRequest {
  DETACH = 0,
  DNLOAD = 1,
  UPLOAD = 2,
  GETSTATUS = 3,
  CLRSTATUS = 4,
  GETSTATE = 5,
  ABORT = 6
}

enum DfuCommand {
  PROG_START = 0x01,
  ERASE = 0x04,
  READ_CONFIG = 0x05,
  START_APP = 0x03
}

enum DfuMemoryUnit {
  FLASH = 0,
  EEPROM = 1
}

// DFU Status Codes
enum DfuStatus {
  OK = 0x00,
  errWRITE = 0x03
}

// DFU State Codes
enum DfuState {
  appIDLE = 0,
  appDETACH = 1,
  dfuIDLE = 2,
  dfuDNLOAD_SYNC = 3,
  dfuDNBUSY = 4
}

interface DfuStatusResponse {
  status: DfuStatus;
  pollTimeout: number;
  state: DfuState;
  string: number; // String index
}

export class DfuDevice extends EventEmitter {
  private _device: USBDevice;
  private interfaceNumber: number;
  private transaction: number = 0;

  private constructor(device: USBDevice, interfaceNumber: number) {
    super();
    this._device = device;
    this.interfaceNumber = interfaceNumber;
  }

  public get device() {
    return this._device;
  }

  static async connect(): Promise<DfuDevice | null> {
    try {
      const device = await navigator.usb.requestDevice({
        filters: [{ vendorId: DFU_VID_AT90USB1286, productId: DFU_PID_AT90USB1286 }]
      });

      await device.open();
      await device.selectConfiguration(1);
      await device.claimInterface(DFU_INTERFACE);

      return new DfuDevice(device, DFU_INTERFACE);
    } catch (err) {
      if (err instanceof DOMException && err.name === 'NotFoundError') {
        return null;
      }
      throw err;
    }
  }

  private async sendDfuCommandOut(
    request: DfuRequest,
    value: number,
    data?: Uint8Array
  ): Promise<void> {
    const result = await this._device.controlTransferOut({
      requestType: 'class',
      recipient: 'interface',
      request,
      value,
      index: this.interfaceNumber,
    }, data);
    if (result.status !== 'ok') {
      throw new Error(`DFU command ${request} failed`);
    }
  }

  private async sendDfuCommandIn(
    request: DfuRequest,
    value: number,
    length: number
  ): Promise<DataView> {
    const result = await this._device.controlTransferIn({
      requestType: 'class',
      recipient: 'interface',
      request,
      value,
      index: this.interfaceNumber,
    }, length);
    if (result.status !== 'ok') {
      throw new Error(`DFU command ${request} failed`);
    }

    return new DataView(result.data!.buffer);
  }

  private async dfuDownload(data: Uint8Array): Promise<void> {
    await this.sendDfuCommandOut(DfuRequest.DNLOAD, this.transaction++, data);
    await this.waitForState(DfuState.dfuIDLE);
  }

  private async getStatus(): Promise<DfuStatusResponse> {
    const view = await this.sendDfuCommandIn(DfuRequest.GETSTATUS, 0, 6);
    return {
      status: view.getUint8(0),
      pollTimeout: view.getUint32(1, true) & 0x00ffffff,
      state: view.getUint8(4),
      string: view.getUint8(5),
    };
  }

  async waitForState(state: DfuState): Promise<void> {
    let status: DfuStatusResponse;
    do {
      status = await this.getStatus();
      if (status.status !== DfuStatus.OK) {
        throw new Error('DFU status error: ' + status.status);
      }
      await new Promise(resolve => setTimeout(resolve, status.pollTimeout));
    } while (status.state !== state);
  }

  private async abort(): Promise<void> {
    console.log('Aborting');
    await this.sendDfuCommandOut(DfuRequest.ABORT, 0);
    await this.waitForState(DfuState.dfuIDLE);
  }

  private async eraseFullChip(): Promise<void> {
    console.log('Erasing full chip');
    this.emit('message', 'Erasing chip...');
    const eraseCmd = new Uint8Array([0x04, 0x00, 0xff]);
    await this.dfuDownload(eraseCmd);
  }

  private async selectMemoryUnit(unit: DfuMemoryUnit): Promise<void> {
    console.log('Selecting memory unit:', unit);
    const selectCmd = new Uint8Array([0x06, 0x03, 0x00, (0xff & unit)]);
    await this.dfuDownload(selectCmd);
  }

  async downloadFirmware(firmware: MemoryMap): Promise<void> {
    await this.abort(); // Necessary to put DFU state to dfuIDLE I think?
    await this.eraseFullChip();
    await this.selectMemoryUnit(DfuMemoryUnit.FLASH);

    this.emit('message', 'Flashing...');
    const fixedFirmware = convertMapToFixedLength(firmware, ATMEL_MAX_TRANSFER_SIZE);
    let i = 0;
    for await (const [address, segment] of fixedFirmware) {
      const progress = i++ / firmware.size;
      console.log('Progress:', progress);
      this.emit('progress', progress);

      const image = createDfuImage(segment, address);
      await this.dfuDownload(image);
    }

    // Send final zero-length packet
    console.log('Sending final packet');
    this.emit('progress', 1);
    await this.dfuDownload(new Uint8Array(0));
    this.emit('message', 'Finished');
  }

  async startApplication(): Promise<void> {
    console.log('Starting application');
    this.emit('message', 'Starting application');
    const startCmd = new Uint8Array([0x04, 0x03, 0x00]);
    await this.sendDfuCommandOut(DfuRequest.DNLOAD, this.transaction++, startCmd);
    try {
      await this.getStatus();
    } catch (_) {
      // For some reason we need to send a GETSTATUS request to get the DFU to actually restart
      // This will raise an exception, so just ignore it
    }
  }
}

// Helper function to create DFU-compatible firmware with suffix
function createDfuImage(segment: Uint8Array, start: number): Uint8Array {
  const header = Buffer.alloc(ATMEL_CONTROL_BLOCK_SIZE);

  header.writeUInt8(DfuCommand.PROG_START, 0);

  // Indicates if the segment is for flash or EEPROM
  header.writeUInt8(0, 1);

  header.writeUInt16BE(0xffff & start, 2);
  header.writeUInt16BE(0xffff & (start + segment.length - 1), 4);

  const suffixLength = 16;
  const suffix = Buffer.alloc(suffixLength);

  // CRC of entire segment including header and suffix
  // Unused?
  suffix.writeUInt32BE(0, 0);

  suffix.writeUInt8(suffixLength, 4);

  // DFU Signature
  suffix.write('DFU', 5);

  // BCD DFU specification number (1.1)
  suffix.writeUInt16BE(0x0110, 8);

  // Vendor ID
  suffix.writeUInt16BE(0xffff, 10);

  // Product ID
  suffix.writeUInt16BE(0xffff, 12);

  // BCD Firmware release number
  suffix.writeUInt16BE(0xffff, 14);

  return Buffer.concat([header, segment, suffix]);
}

// Ideally this wouldn't be required, but there's a bug in the nrf-intel-hex library
// and how it handles the chunking of the firmware with variable length segments.
// Basically, it doesn't seem to be able to handle segments that aren't a multiple of 16 bytes correctly
// and will cause some segments to exceed their supposed max block size.
// Thus, we do a third pass to make sure each segment fits into the max block size,
// and to shift any overflows into the next block.
// I'm not sure if this is actually a bug in regards to the Intel Hex spec,
// but it causes issues with the Atmel DFU protocol.
function convertMapToFixedLength(inputMap: MemoryMap, fixedLength: number) {
  const outputMap = new Map();
  let currentChunk = new Uint8Array(fixedLength);
  let offset = 0;       // current write index in the chunk
  let address = 0;   // key for the output map

  // Iterate over the input map in insertion order.
  for (const [_, arr] of inputMap) {
    for (let i = 0; i < arr.length; i++) {
      // Write the byte into the current chunk.
      currentChunk[offset] = arr[i];
      offset++;

      // If we have filled the chunk, add it to the output map
      if (offset === fixedLength) {
        outputMap.set(address, currentChunk);
        // Move to the next address.
        address += fixedLength;
        // Create a new chunk and reset offset.
        currentChunk = new Uint8Array(fixedLength);
        offset = 0;
      }
    }
  }

  // If there are remaining bytes that didn't fill a complete chunk,
  // we pad the rest with 0xff.
  if (offset > 0) {
    currentChunk.fill(0xff, offset);
    // Pad to the nearest 16th byte
    offset += 16 - (16 % offset);
    currentChunk = currentChunk.slice(0, offset);
    outputMap.set(address, currentChunk);
  }

  return outputMap;
}