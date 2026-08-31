# vscp-guid

VSCP GUID parsing and formatting for Node.js. Provides the `VscpGuid` class used to parse, format and
manipulate 16-byte VSCP GUIDs, mirroring the behavior of the C++ `cguid` class from the VSCP daemon.

## Installation

```bash
npm install ./src/vscp/nodejs/vscp-guid
```

## Usage

```javascript
const { VscpGuid, createGuid } = require('vscp-guid');

const guid = new VscpGuid('FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00');
console.log(guid.toString());       // FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00
console.log(guid.toStringUUID());   // FFEEDDCC-BBAA-9988-7766-554433221100

const empty = createGuid();
empty.setClientID(0xabcd);
```

## Testing

```bash
npm test
```

## License

MIT License, see [LICENSE](LICENSE).
