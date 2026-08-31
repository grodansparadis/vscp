'use strict';

const GUID_SIZE = 16;

function isHexDigit(character) {
    return (character >= '0' && character <= '9') ||
        (character >= 'a' && character <= 'f') ||
        (character >= 'A' && character <= 'F');
}

function countHexDigits(value, position) {
    let count = 0;
    while (position + count < value.length && isHexDigit(value[position + count])) {
        count++;
    }
    return count;
}

function isSeparator(character) {
    return character === ':' || character === '-' || character === ',';
}

class VscpGuid {
    constructor(value) {
        this._guid = new Uint8Array(GUID_SIZE);

        if (typeof value === 'string') {
            this.getFromString(value);
        } else if (value instanceof VscpGuid) {
            this.getFromArray(value.getGUID());
        } else if (value !== undefined && value !== null) {
            this.getFromArray(value);
        }
    }

    clear() {
        this._guid.fill(0);
    }

    getFromString(value) {
        const text = String(value);
        let position = 0;

        this.clear();

        while (text[position] === ' ' || text[position] === '\t') {
            position++;
        }

        if (text[position] === '{') {
            position++;
            while (text[position] === ' ' || text[position] === '\t') {
                position++;
            }
        }

        if (position >= text.length) {
            return;
        }

        if (text[position] === '-') {
            if (text[position + 1] === ':') {
                this._parsePrefixedGroups(text, position + 2, 0x00);
                return;
            }
            if (position + 1 >= text.length || !isHexDigit(text[position + 1])) {
                return;
            }
        }

        if (text[position] === ':' && text[position + 1] === ':') {
            if (position + 2 >= text.length || !isHexDigit(text[position + 2])) {
                this._guid.fill(0xff);
                return;
            }
            this._parsePrefixedGroups(text, position + 2, 0xff);
            return;
        }

        let guidIndex = 0;
        while (position < text.length && guidIndex < GUID_SIZE) {
            if (!isHexDigit(text[position])) {
                break;
            }

            const hexLength = countHexDigits(text, position);
            const hexGroup = text.slice(position, position + hexLength);

            if (hexLength <= 2) {
                this._guid[guidIndex++] = parseInt(hexGroup, 16);
            } else if (hexLength <= 4) {
                const parsed = parseInt(hexGroup, 16);
                if (guidIndex + 1 < GUID_SIZE) {
                    this._guid[guidIndex++] = (parsed >>> 8) & 0xff;
                    this._guid[guidIndex++] = parsed & 0xff;
                } else {
                    this._guid[guidIndex++] = parsed & 0xff;
                }
            } else {
                const bytesToParse = Math.min(Math.ceil(hexLength / 2), GUID_SIZE - guidIndex);
                for (let index = 0; index < bytesToParse && guidIndex < GUID_SIZE; index++) {
                    const high = parseInt(text[position++], 16);
                    const low = isHexDigit(text[position]) ? parseInt(text[position++], 16) : 0;
                    this._guid[guidIndex++] = (high << 4) | low;
                }
                if (guidIndex < GUID_SIZE && isSeparator(text[position])) {
                    position++;
                }
                continue;
            }

            position += hexLength;
            if (guidIndex < GUID_SIZE && isSeparator(text[position])) {
                position++;
            }
        }
    }

    getFromArray(value) {
        for (let index = 0; index < GUID_SIZE; index++) {
            this._guid[index] = Number(value[index]) & 0xff;
        }
    }

    toString() {
        return Array.from(this._guid, byte => byte.toString(16).padStart(2, '0').toUpperCase()).join(':');
    }

    toStringCompact() {
        let firstNonFf = 0;
        while (firstNonFf < GUID_SIZE && this._guid[firstNonFf] === 0xff) {
            firstNonFf++;
        }

        if (firstNonFf === GUID_SIZE) {
            return '::';
        }
        if (firstNonFf === 0) {
            return this.toString();
        }

        return `::${Array.from(this._guid.slice(firstNonFf), byte => byte.toString(16).padStart(2, '0').toUpperCase()).join(':')}`;
    }

    toStringUUID() {
        const hex = Array.from(this._guid, byte => byte.toString(16).padStart(2, '0').toUpperCase()).join('');
        return `${hex.slice(0, 8)}-${hex.slice(8, 12)}-${hex.slice(12, 16)}-${hex.slice(16, 20)}-${hex.slice(20)}`;
    }

    getAsString() {
        return this.toString();
    }

    getGUID(position) {
        return position === undefined ? this._guid : this.getAt(position);
    }

    getAt(position) {
        return this._guid[Number(position) & 0x0f];
    }

    setAt(position, value) {
        this._guid[Number(position) & 0x0f] = Number(value) & 0xff;
    }

    getLSB() {
        return this._guid[15];
    }

    setLSB(value) {
        this._guid[15] = Number(value) & 0xff;
    }

    getMSB() {
        return this._guid[0];
    }

    setMSB(value) {
        this._guid[0] = Number(value) & 0xff;
    }

    getNicknameID() {
        return (this._guid[14] << 8) + this._guid[15];
    }

    getNickname() {
        return this._guid[15];
    }

    writeGUID(destination) {
        if (destination == null) {
            return;
        }
        for (let index = 0; index < GUID_SIZE; index++) {
            destination[index] = this._guid[index];
        }
    }

    reverse() {
        this._guid.reverse();
    }

    writeGUID_reverse(destination) {
        if (destination == null) {
            return;
        }
        for (let index = 0; index < GUID_SIZE; index++) {
            destination[15 - index] = this._guid[index];
        }
    }

    isSameGUID(value) {
        if (value == null) {
            return false;
        }
        for (let index = 0; index < GUID_SIZE; index++) {
            if (this._guid[index] !== (Number(value[index]) & 0xff)) {
                return false;
            }
        }
        return true;
    }

    isNULL() {
        return this._guid.every(byte => byte === 0);
    }

    setClientID(clientId) {
        this._guid[12] = (Number(clientId) >>> 8) & 0xff;
        this._guid[13] = Number(clientId) & 0xff;
    }

    getClientID() {
        return (this._guid[12] << 8) + this._guid[13];
    }

    setNicknameID(nicknameId) {
        this._guid[14] = (Number(nicknameId) >>> 8) & 0xff;
        this._guid[15] = Number(nicknameId) & 0xff;
    }

    equals(other) {
        return other instanceof VscpGuid && this.isSameGUID(other.getGUID());
    }

    notEquals(other) {
        return !this.equals(other);
    }

    _parsePrefixedGroups(text, position, padding) {
        const bytes = [];

        while (position < text.length && bytes.length < GUID_SIZE) {
            if (!isHexDigit(text[position])) {
                break;
            }

            const hexLength = countHexDigits(text, position);
            let parsedLength = hexLength;
            if (hexLength <= 2) {
                bytes.push(parseInt(text.slice(position, position + hexLength), 16));
            } else if (hexLength <= 4) {
                const parsed = parseInt(text.slice(position, position + hexLength), 16);
                bytes.push((parsed >>> 8) & 0xff, parsed & 0xff);
            } else {
                const byteCount = Math.min(Math.ceil(hexLength / 2), 4);
                parsedLength = Math.min(hexLength, byteCount * 2);
                const parsed = parseInt(text.slice(position, position + parsedLength), 16);
                for (let index = byteCount - 1; index >= 0 && bytes.length < GUID_SIZE; index--) {
                    bytes.push((parsed >>> (index * 8)) & 0xff);
                }
            }

            position += parsedLength;
            if (isSeparator(text[position])) {
                position++;
            }
        }

        const start = GUID_SIZE - Math.min(bytes.length, GUID_SIZE);
        this._guid.fill(padding, 0, start);
        this._guid.set(bytes.slice(0, GUID_SIZE), start);
    }
}

function createGuid(value) {
    return new VscpGuid(value);
}

module.exports = { VscpGuid, createGuid };