/**
 * TypeScript definitions for VSCP Level 2 Client Interface
 * 
 * Copyright (C) 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>
 * MIT License
 */

import { EventEmitter } from 'events';

// VSCP Constants
export interface VscpConstants {
    readonly DEFAULT_TCP_PORT: 9598;
    readonly DEFAULT_UDP_PORT: 33333;
    readonly DEFAULT_MULTICAST_PORT: 44444;
    readonly MAX_DATA: 512;
    readonly SIZE_GUID: 16;
    readonly LEVEL1: 0;
    readonly LEVEL2: 1;
    
    // Priorities
    readonly PRIORITY_0: 0x00;
    readonly PRIORITY_1: 0x20;
    readonly PRIORITY_2: 0x40;
    readonly PRIORITY_3: 0x60;
    readonly PRIORITY_4: 0x80;
    readonly PRIORITY_5: 0xA0;
    readonly PRIORITY_6: 0xC0;
    readonly PRIORITY_7: 0xE0;
    readonly PRIORITY_HIGH: 0x00;
    readonly PRIORITY_NORMAL: 0x60;
    readonly PRIORITY_MEDIUM: 0xC0;
    readonly PRIORITY_LOW: 0xE0;
    
    // Header flags
    readonly HEADER_PRIORITY_MASK: 0xE0;
    readonly HEADER_HARD_CODED: 0x10;
    readonly HEADER_NO_CRC: 0x08;
    readonly HEADER16_DUMB: 0x8000;
    readonly HEADER16_IPV6_GUID: 0x1000;
    
    // GUID Types
    readonly HEADER16_GUID_TYPE_STANDARD: 0x0000;
    readonly HEADER16_GUID_TYPE_IPV6: 0x1000;
    readonly HEADER16_GUID_TYPE_RFC4122V1: 0x2000;
    readonly HEADER16_GUID_TYPE_RFC4122V4: 0x3000;
    
    // Error codes
    readonly ERROR_SUCCESS: 0;
    readonly ERROR_GENERAL: -1;
    readonly ERROR_INVALID_HANDLE: -2;
    readonly ERROR_INVALID_PARAMETER: -3;
    readonly ERROR_TIMEOUT: -4;
    readonly ERROR_NOT_CONNECTED: -5;
    readonly ERROR_WRITE_ERROR: -6;
    readonly ERROR_READ_ERROR: -7;
    readonly ERROR_INVALID_POINTER: -8;
    readonly ERROR_OPERATION_FAILED: -9;
    
    // Flags
    readonly FLAG_ENABLE_DEBUG: 0x80000000;
    readonly DEFAULT_RESPONSE_TIMEOUT: 3000;
}

export declare const VSCP_CONSTANTS: VscpConstants;

export type VscpGuidValue = VscpGuid | ArrayLike<number> | string;

export declare class VscpGuid {
    constructor(value?: VscpGuidValue);
    clear(): void;
    getFromString(value: string): void;
    getFromArray(value: ArrayLike<number>): void;
    toString(): string;
    toStringCompact(): string;
    toStringUUID(): string;
    getAsString(): string;
    getGUID(): Uint8Array;
    getGUID(position: number): number;
    getAt(position: number): number;
    setAt(position: number, value: number): void;
    getLSB(): number;
    setLSB(value: number): void;
    getMSB(): number;
    setMSB(value: number): void;
    getNicknameID(): number;
    getNickname(): number;
    writeGUID(destination: ArrayLike<number>): void;
    reverse(): void;
    writeGUID_reverse(destination: ArrayLike<number>): void;
    isSameGUID(value: ArrayLike<number> | null): boolean;
    isNULL(): boolean;
    setClientID(clientId: number): void;
    getClientID(): number;
    setNicknameID(nicknameId: number): void;
    equals(other: VscpGuid): boolean;
    notEquals(other: VscpGuid): boolean;
}

// VSCP Event interface
export interface VscpEventOptions {
    crc?: number;
    obid?: number;
    year?: number;
    month?: number;
    day?: number;
    hour?: number;
    minute?: number;
    second?: number;
    timestamp?: number;
    head?: number;
    vscp_class?: number;
    vscp_type?: number;
    GUID?: number[];
    sizeData?: number;
    data?: number[];
}

export declare class VscpEvent {
    crc: number;
    obid: number;
    year: number;
    month: number;
    day: number;
    hour: number;
    minute: number;
    second: number;
    timestamp: number;
    head: number;
    vscp_class: number;
    vscp_type: number;
    GUID: number[];
    sizeData: number;
    data: number[];
    
    constructor(options?: VscpEventOptions);
    
    toJSON(): VscpEventOptions;
    static fromJSON(json: VscpEventOptions): VscpEvent;
    getPriority(): number;
    setPriority(priority: number): void;
    isHardCoded(): boolean;
    setHardCoded(hardCoded: boolean): void;
    setCurrentTime(): void;
}

// VSCP Event Filter interface
export interface VscpEventFilterOptions {
    filter_priority?: number;
    mask_priority?: number;
    filter_class?: number;
    mask_class?: number;
    filter_type?: number;
    mask_type?: number;
    filter_GUID?: number[];
    mask_GUID?: number[];
}

export declare class VscpEventFilter {
    filter_priority: number;
    mask_priority: number;
    filter_class: number;
    mask_class: number;
    filter_type: number;
    mask_type: number;
    filter_GUID: number[];
    mask_GUID: number[];
    
    constructor(options?: VscpEventFilterOptions);
    
    clear(): void;
    setClassAndType(vscp_class: number, vscp_type: number): void;
}

// Version information
export interface VersionInfo {
    major: number;
    minor: number;
    release: number;
    build: number;
}

// Statistics interface
export interface Statistics {
    eventsReceived: number;
    eventsSent: number;
    errors: number;
    connectTime: Date | null;
}

// Response interfaces
export interface CountResponse {
    error: number;
    count: number;
}

export interface VersionResponse {
    error: number;
    version: VersionInfo;
}

export interface InterfacesResponse {
    error: number;
    interfaces: string[];
}

export interface CapabilitiesResponse {
    error: number;
    capabilities: number;
}

export interface ReceiveResponse {
    error: number;
    event: VscpEvent | null;
}

// Client options interface
export interface VscpLevel2ClientOptions {
    interface?: string;
    flags?: number;
    responseTimeout?: number;
    connectionTimeout?: number;
    maxQueueSize?: number;
}

// VSCP Level 2 Client class
export declare class VscpLevel2Client extends EventEmitter {
    interface: string;
    flags: number;
    responseTimeout: number;
    connectionTimeout: number;
    
    constructor(options?: VscpLevel2ClientOptions);
    
    // Core methods from level2drvdef.h interface
    init(interface: string, flags?: number, timeout?: number): number;
    connect(): Promise<number>;
    disconnect(): Promise<number>;
    isConnected(): boolean;
    send(event: VscpEvent, timeout?: number): Promise<number>;
    receive(timeout?: number): Promise<ReceiveResponse>;
    
    // Additional methods from vscpClientLevel2
    setFilter(filter: VscpEventFilter): number;
    getCount(): Promise<CountResponse>;
    clear(): number;
    getVersion(): Promise<VersionResponse>;
    getInterfaces(): Promise<InterfacesResponse>;
    getCapabilities(): Promise<CapabilitiesResponse>;
    
    // Configuration methods
    getConfigAsJson(): string;
    initFromJson(config: string): boolean;
    setConnectionTimeout(timeout: number): void;
    getConnectionTimeout(): number;
    setResponseTimeout(timeout: number): void;
    getResponseTimeout(): number;
    
    // Utility methods
    getStatistics(): Statistics;
    injectEvent(event: VscpEvent): void;
    
    // Event emitter methods
    on(event: 'connected', listener: () => void): this;
    on(event: 'disconnected', listener: () => void): this;
    on(event: 'eventSent', listener: (event: VscpEvent) => void): this;
    on(event: 'eventReceived', listener: (event: VscpEvent) => void): this;
    on(event: 'newEvent', listener: (event: VscpEvent) => void): this;
    on(event: 'error', listener: (error: Error) => void): this;
    
    emit(event: 'connected'): boolean;
    emit(event: 'disconnected'): boolean;
    emit(event: 'eventSent', event: VscpEvent): boolean;
    emit(event: 'eventReceived', event: VscpEvent): boolean;
    emit(event: 'newEvent', event: VscpEvent): boolean;
    emit(event: 'error', error: Error): boolean;
}

// Library info interface
export interface LibraryInfo {
    name: string;
    version: string;
    description: string;
    author: string;
    license: string;
    homepage: string;
}

// Factory functions
export declare function createClient(options?: VscpLevel2ClientOptions): VscpLevel2Client;
export declare function createEvent(options?: VscpEventOptions): VscpEvent;
export declare function createFilter(options?: VscpEventFilterOptions): VscpEventFilter;
export declare function createGuid(value?: VscpGuidValue): VscpGuid;

// Aliases
export declare const Client: typeof VscpLevel2Client;
export declare const Event: typeof VscpEvent;
export declare const Filter: typeof VscpEventFilter;
export declare const Constants: VscpConstants;

// Version and library info
export declare const version: string;
export declare const info: LibraryInfo;