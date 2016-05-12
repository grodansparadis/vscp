// settings.js
// ===========
// This file contains settings for VSCP websocket and other JavaScript examples
//

var settings = {

    user: "admin",
    authdomain: "mydomain.com",
    passwordHash: "d50c3180375c27927c22e42a379c3f67", // Hash on "user:authdomain:password"
    // url: "ws://192.168.1.9:8080" // Non SSL (to be able to use from remote machine)
    //url: "wss://192.168.1.9:8080" // SSL (to be able to use from remote machine)
    url: "ws://demo.vscp.org:8080" // Demoserver
};
