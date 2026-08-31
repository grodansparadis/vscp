/**
 * TypeScript definitions for the VSCP GUID library
 *
 * Copyright (C) 2000-2026 Ake Hedman, the VSCP project <info@vscp.org>
 * MIT License
 */

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

export declare function createGuid(value?: VscpGuidValue): VscpGuid;
