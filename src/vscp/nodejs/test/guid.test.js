'use strict';

const assert = require('assert');
const { Guid, VscpGuid, createGuid } = require('../lib');

const expected = [0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, 0x99, 0x88,
    0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0x00];

const guid = new VscpGuid('{FFEEDDCC-BBAA-9988-7766-55443322-1100}');
assert.deepStrictEqual(Array.from(guid.getGUID()), expected);
assert.strictEqual(guid.toString(), 'FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00');
assert.strictEqual(guid.toStringUUID(), 'FFEEDDCC-BBAA-9988-7766-554433221100');

const grouped = new Guid('FFEEDD:CCBBAA99-8877,665544332211:00');
assert.deepStrictEqual(Array.from(grouped.getGUID()), expected);
assert.ok(guid.equals(grouped));
assert.ok(!guid.notEquals(grouped));

const compact = new VscpGuid('::0102:03aa:44:01:30');
assert.strictEqual(compact.toStringCompact(), '::01:02:03:AA:44:01:30');
assert.strictEqual(compact.getAt(16), 0xff);
compact.setAt(31, 0x42);
assert.strictEqual(compact.getLSB(), 0x42);

const identifiers = createGuid();
identifiers.setClientID(0xabcd);
identifiers.setNicknameID(0x1234);
assert.strictEqual(identifiers.getClientID(), 0xabcd);
assert.strictEqual(identifiers.getNicknameID(), 0x1234);

const reversed = new Uint8Array(16);
guid.writeGUID_reverse(reversed);
assert.strictEqual(reversed[0], 0x00);
assert.strictEqual(reversed[15], 0xff);