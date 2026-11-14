// vscp_client_tcp.h
//
// tcp/ip client communication classes.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version
// 2 of the License, or (at your option) any later version.
//
// This file is part of the VSCP (https://www.vscp.org)
//
// Copyright:  (C) 2007-2026
// Ake Hedman, the VSCP project, <info@vscp.org>
//
// This file is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this file see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//

#if !defined(VSCPCLIENTTCP_H__INCLUDED_)
#define VSCPCLIENTTCP_H__INCLUDED_

#include "vscp-client-base.h"
#include <vscpremotetcpif.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>

// Max number of events held in the worker receive queue (m_receiveList)
#define VSCP_TCP_CLIENT_MAX_RECEIVE_QUEUE 2000

class vscpClientTcp : public CVscpClient {

public:
  enum class tls_mode
  {
    auto_select = 0, // Select TLS based on endpoint prefix (stcp://)
    force_tls,       // Always use TLS
    force_plain      // Never use TLS
  };

  /// A physical binary connection; both values use send in one-connection modes.
  enum class binary_channel
  {
    send,
    receive
  };

  vscpClientTcp();
  virtual ~vscpClientTcp();

  /*!
      Initialize the tcp client
      @param strHostname Hostname and port for remote host. Can be prefixed with
          tcp:// or stcp:// (SSL connection)
      @param strUsername Username used to login on remote host.
      @param strPassword Password used to login on remote host.
      @param bPolling If true, use one connection and poll for events. If false,
          use separate send and receive connections unless single-channel mode
          is enabled. Polling and single-channel mode cannot be enabled together.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  int init(const std::string &strHostname = "tcp://localhost:9598",
           const std::string &strUsername = "vscp",
           const std::string &strPassword = "secret",
           bool bPolling                  = false);

  /*!
      Enable or disable asynchronous single-channel mode. Configure it while
      disconnected and with polling disabled. The default is disabled.
      In text mode, command replies must be distinguishable from bare +OK
      receive-loop keepalives. Binary mode uses OPEN instead of RCVLOOP.
      @return VSCP_ERROR_SUCCESS, or VSCP_ERROR_PARAMETER if the mode cannot
          be changed or conflicts with polling.
  */
  int setSingleChannelReceiveLoop(bool enable = true);

  bool isSingleChannelReceiveLoopEnabled(void) const { return m_bSingleChannelReceiveLoop; }

  /*!
      Enable or disable VSCP-GBP binary mode for event and raw-frame APIs.
      Configure while disconnected; binary mode is disabled by default.
      Asynchronous modes negotiate binary OPEN; polling uses binary RETR.
      @return VSCP_ERROR_SUCCESS, or VSCP_ERROR_PARAMETER if active.
  */
  int setBinaryMode(bool enable = true);
  bool isBinaryModeEnabled(void) const { return m_bBinaryMode; }

  /*!
      Send one complete VSCP-GBP frame (1-8191 bytes) without its TCP stream
      prefix. Binary mode adds the [0x55][version:3 | length:13] prefix. In a single-connection
      mode both channel values refer to the send connection. Encrypted frames
      are opaque; this method does not encrypt, decrypt, or validate their CRC.
      Calling either raw API enables delivery of subsequent event confirmations
      to the raw queue, which the application must drain.

      While binary polling is active, raw RETR may request at most one event.
      A raw RETR or CLEAR that conflicts with an internal request returns
      VSCP_ERROR_ALREADY_DEFINED. Such tracked commands require a valid CRC;
      if their reply does not arrive before the response timeout, the receiver
      stops to avoid misrouting a later reply. Encrypted replies to encrypted
      commands must remain encrypted if their command code could conflict with
      internal requests, since the client cannot identify an encrypted command.
      @return VSCP_ERROR_SUCCESS or a VSCP error code.
  */
  int sendBinaryFrame(const std::vector<uint8_t> &frame, binary_channel channel = binary_channel::send);

  /*!
      Receive a raw command, reply, or encrypted frame without its TCP length
      prefix. Unencrypted events are delivered through the event API; successful
      event confirmations are delivered here only after a raw API is used.
      The optional channel output identifies the source connection, or send
      in one-connection modes. The raw queue is unbounded and must be drained.
      A zero timeout returns VSCP_ERROR_RCV_EMPTY without waiting; a positive
      timeout returns VSCP_ERROR_TIMEOUT on expiry. Negative values are rejected.
      The output arguments remain unchanged when no frame is returned.
      @return VSCP_ERROR_SUCCESS or a VSCP error code.
  */
  int receiveBinaryFrame(std::vector<uint8_t> &frame,
                         long timeout = 100,
                         binary_channel *channel = nullptr);

  /*!
      Set receive callbacks before connecting in single-channel or binary mode.
      Changing them while either mode is active returns VSCP_ERROR_NOT_SUPPORTED.
  */
  int setCallbackEv(std::function<void(vscpEvent &ev, void *pobj)> callback,
                    void *pData = nullptr) override;
  int setCallbackEx(std::function<void(vscpEventEx &ex, void *pobj)> callback,
                    void *pData = nullptr) override;

  /*!
      Connect to remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int connect(void);

  /*!
      Disconnect from remote host
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int disconnect(void);

  /*!
      Check if connected.
      @return true if connected, false otherwise.
  */
  virtual bool isConnected(void);

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEvent &ev);

  /*!
      Send VSCP event to remote host.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int send(vscpEventEx &ex);

  /*!
    Send VSCP CAN(AL) message to remote host.
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
*/
  virtual int send(canalMsg &msg);

  /*!
      Receive VSCP event from remote host
      In single-channel or binary mode this reads the local event queue and returns
      VSCP_ERROR_RCV_EMPTY when no event is available. Queued receives are
      unavailable while a receive callback is active.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEvent &ev);

  /*!
      Blocking receive of VSCP event ex from remote host
      @param ev VSCP event ex that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
          In single-channel or binary mode, zero does not wait and negative values
          are rejected. Blocking receives are unavailable with callbacks.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(vscpEvent &ev, long timeout = 100);

  /*!
      Receive VSCP event ex from remote host
      Single-channel and binary modes use the same local queue as receive(vscpEvent &).
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receive(vscpEventEx &ex);

  /*!
      Blocking receive of VSCP event ex from remote host
      @param ex VSCP event ex that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
          In single-channel or binary mode, zero does not wait and negative values
          are rejected. Blocking receives are unavailable with callbacks.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(vscpEventEx &ex, long timeout = 100);

  /*!
    Receive CAN(AL) message from remote host
    Single-channel and binary modes use the same local queue as receive(vscpEvent &).
    @return Return VSCP_ERROR_SUCCESS of OK and error code else.
*/
  virtual int receive(canalMsg &msg);

  /*!
      Receive blocking CAN(AL) message from remote host
      @param msg CANAL message that will get the result.
      @param timeout Timeout in milliseconds. Default is 100 ms.
          In single-channel or binary mode, zero does not wait and negative values
          are rejected. Blocking receives are unavailable with callbacks.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int receiveBlocking(canalMsg &msg, long timeout = 100);

  /*!
      Set interface filter
      In single-channel or binary mode the filter is applied locally to future events.
      @param filter VSCP Filter to set.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int setfilter(vscpEventFilter &filter);

  /*!
      Get number of events waiting to be received on remote
      interface. In single-channel or binary mode this is the local receive queue size.
      @param pcount Pointer to an unsigned integer that get the count of events.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getcount(uint16_t *pcount);

  /*!
      Clear the input queue. In single-channel or binary mode this requests a
      server clear and discards locally queued events before its acknowledgement.
      In-flight events and callbacks already running may still be delivered.
      Synchronous binary clear from a receive callback is not supported.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int clear(void);

  /*!
      Get version from interface
      This command is not available in single-channel text or binary mode.
      @param pmajor Pointer to uint8_t that get major version of interface.
      @param pminor Pointer to uint8_t that get minor version of interface.
      @param prelease Pointer to uint8_t that get release version of interface.
      @param pbuild Pointer to uint8_t that get build version of interface.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getversion(uint8_t *pmajor, uint8_t *pminor, uint8_t *prelease, uint8_t *pbuild);

  /*!
      Get interfaces
      This command is not available in single-channel text or binary mode.
      @param iflist Get a list of available interfaces
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getinterfaces(std::deque<std::string> &iflist);

  /*!
      Get capabilities (wcyd) from remote interface
      This command is not available in single-channel text or binary mode.
      @return Return VSCP_ERROR_SUCCESS of OK and error code else.
  */
  virtual int getwcyd(uint64_t &wcyd);

  /*!
      Return a JSON representation of connection
      @return JSON representation as string
  */
  virtual std::string getConfigAsJson(void);

  /*!
      Set member variables from JSON representation of connection
      The optional booleans "single-channel" and "binary-mode" select the
      corresponding modes. "single-channel" cannot be combined with "bpoll".
      @param config JSON representation as string
      @return True on success, false on failure.
  */
  virtual bool initFromJson(const std::string &config);

  /*!
      Getter/setters for connection timeout
      Time is in milliseconds
  */
  virtual void setConnectionTimeout(uint32_t timeout);
  virtual uint32_t getConnectionTimeout(void);

  /*!
      Getter/setters for response timeout
      Time is in milliseconds
  */
  virtual void setResponseTimeout(uint32_t timeout);
  virtual uint32_t getResponseTimeout(void);

  void setTLSMode(tls_mode mode)
  {
    m_tlsMode = mode;
    if (tls_mode::force_tls == m_tlsMode) {
      m_bTLS = true;
    }
    else if (tls_mode::force_plain == m_tlsMode) {
      m_bTLS = false;
    }
  };

  tls_mode getTLSMode(void) const { return m_tlsMode; };

  void enableTLS(bool bEnable = true)
  {
    setTLSMode(bEnable ? tls_mode::force_tls : tls_mode::force_plain);
  };

  void setTLSAutoSelect(void)
  {
    m_tlsMode = tls_mode::auto_select;
    m_bTLS    = false;
  };

  void setTLSOptions(bool bVerifyPeer,
                     const std::string &cafile = "",
                     const std::string &capath = "",
                     const std::string &certfile = "",
                     const std::string &keyfile = "",
                     const std::string &pwKeyfile = "")
  {
    m_tlsMode     = tls_mode::force_tls;
    m_bTLS        = true;
    m_bVerifyPeer = bVerifyPeer;
    m_cafile = cafile;
    m_capath = capath;
    m_certfile = certfile;
    m_keyfile = keyfile;
    m_pwKeyfile = pwKeyfile;
  };

  // Init TCP CaFile
  void setTlsCaFile(const std::string &cafile)
  {
    m_tlsMode = tls_mode::force_tls;
    m_bTLS    = true;
    m_cafile  = cafile;
  };

  // Init TCP CaPath
  void setTlsCaPath(const std::string &capath)
  {
    m_tlsMode = tls_mode::force_tls;
    m_bTLS    = true;
    m_capath = capath;
  };

  // Init TCP CertFile
  void setTlsCertFile(const std::string &certfile)
  {
    m_tlsMode   = tls_mode::force_tls;
    m_bTLS      = true;
    m_certfile = certfile;
  };

  // Init TCP KeyFile
  void setTlsKeyFile(const std::string &keyfile)
  {
    m_tlsMode  = tls_mode::force_tls;
    m_bTLS     = true;
    m_keyfile = keyfile;
  };

  // Init TCP PwKeyFile
  void setTlsPwKeyFile(const std::string &pwkeyfile)
  {
    m_tlsMode    = tls_mode::force_tls;
    m_bTLS       = true;
    m_pwKeyfile = pwkeyfile;
  };

  // Backward-compatible aliases (legacy naming)
  void setMqttCaFile(const std::string &cafile) { setTlsCaFile(cafile); };
  void setMqttCaPath(const std::string &capath) { setTlsCaPath(capath); };
  void setMqttCertFile(const std::string &certfile) { setTlsCertFile(certfile); };
  void setMqttKeyFile(const std::string &keyfile) { setTlsKeyFile(keyfile); };
  void setMqttPwKeyFile(const std::string &pwkeyfile) { setTlsPwKeyFile(pwkeyfile); };

  // ------------------------------------------------------------------------
  //  Receive worker thread functions
  // ------------------------------------------------------------------------

  /*!
      Get the tcp/ip receive object. Single-channel mode and binary polling
      return the shared connection. Do not operate on it while the client runs.
      @return Pointer to receive tcp/ip object or null if not defined
  */
  VscpRemoteTcpIf *getTcpReceive(void)
  {
    return (m_bSingleChannelReceiveLoop || (m_bBinaryMode && m_bPolling)) ? &m_tcp : &m_tcpReceive;
  };

  /*!
      Send event to defined callbacks if any is defined
      @param pev Pointer to VSCP event
  */
  void sendToCallbacks(vscpEvent *pev);

  /*!
      Fetch one event from the server on the main connection.
      Used by the poll worker thread. Caller must hold m_mutexTcpIpObject.
      @param ev Event that will get the received event
      @return VSCP_ERROR_SUCCESS if an event was received, error code else.
  */
  int pollServerReceive(vscpEvent &ev);

public:
  /// Flag for worker thread run as long it's true
  bool m_bRun;

  // Event lists
  // std::list<vscpEvent *> m_sendList;
  std::list<vscpEvent *> m_receiveList;

  /// Mutex to protect receive tcp/ip object
  pthread_mutex_t m_mutexTcpIpObject;

  /// Mutex to protect receive queue
  pthread_mutex_t m_mutexReceiveQueue;

  /*!
    Event object to indicate that there is an event in the
    output queue
  */
#ifdef WIN32
  HANDLE m_semReceiveQueue;
#else
  sem_t m_semReceiveQueue;
#endif

  /// Filters for input/output
  vscpEventFilter m_filterIn;
  vscpEventFilter m_filterOut;

  /// Used for channel id (prevent sent events from being received)
  uint32_t m_obid;

private:
  /*!
      Take the oldest event from the worker receive queue (m_receiveList).
      Ownership of the event data moves to ev.
      @param ev Event that will get the queued event
      @param bConsumeSignal True to also take one count from the receive
          semaphore (non-blocking receive). False when the caller already
          waited on the semaphore.
      @return VSCP_ERROR_SUCCESS or VSCP_ERROR_RCV_EMPTY if queue is empty.
  */
  int popReceiveList(vscpEvent &ev, bool bConsumeSignal);

  /// Free all events in the worker receive queue
  void clearReceiveList(void);

  /*!
      The main interface is always opened. The receive interface is opened
      only in two-channel mode. Polling and single-channel modes use the main
      interface for both sending and receiving.
  */

  /// Sending interface
  VscpRemoteTcpIf m_tcp;

  /// Receiving interface
  VscpRemoteTcpIf m_tcpReceive;

  // Interface on remote host
  cguid m_guidif;

  // ------------------------------------------------------------------------

  /// Workerthread
  std::thread *m_pworkerthread;

  // Connection parameters set by init

  /// Remote host to connect to
  std::string m_strHostname;

  /// Port of remote host
  short m_port;

  /// Username to login with at remote host
  std::string m_strUsername;

  /// Password to login with at remote host
  std::string m_strPassword;

  /// If true the remote host interface will be polled.
  bool m_bPolling;

  /// If true the main interface runs an asynchronous single-channel receive loop.
  bool m_bSingleChannelReceiveLoop;

  /// If true connections use VSCP-GBP binary frames.
  bool m_bBinaryMode;

  bool usesManagedReceive(void) const { return m_bSingleChannelReceiveLoop || m_bBinaryMode; }
  bool binaryUsesTwoChannels(void) const { return !m_bPolling && !m_bSingleChannelReceiveLoop; }
  binary_channel normalizeBinaryChannel(binary_channel channel) const;
  VscpRemoteTcpIf &binaryInterface(binary_channel channel);
  bool isReceiveWorkerThread(void);
  int connectBinary(void);
  int openBinaryReceiveChannel(VscpRemoteTcpIf &interface, std::deque<std::vector<uint8_t>> &buffered);
  int binaryCommand(binary_channel channel, uint16_t command, std::vector<uint8_t> &reply);
  int dispatchBinaryEvent(binary_channel channel, const std::vector<uint8_t> &frame, bool acknowledge);
  int processBinaryFrame(binary_channel channel, const std::vector<uint8_t> &frame);
  void binaryWorker(binary_channel channel, std::deque<std::vector<uint8_t>> buffered);

  void singleChannelWorker(void);
  void failSingleChannelWorker(int error);
  int receiveSingleChannel(vscpEventEx &event, long timeout, bool blocking);

  std::atomic<bool> m_singleChannelRunning{false};
  std::thread m_singleChannelWorkerThread;
  std::mutex m_singleChannelCommandMutex;
  std::mutex m_singleChannelDispatchMutex;
  std::condition_variable m_singleChannelDispatchCondition;
  std::atomic<bool> m_singleChannelPauseRequested{false};
  std::mutex m_singleChannelMutex;
  std::condition_variable m_singleChannelCondition;
  std::deque<vscpEventEx> m_singleChannelEvents;
  std::thread::id m_singleChannelWorkerId;
  int m_singleChannelError = VSCP_ERROR_NOT_CONNECTED;

  struct raw_binary_frame {
    binary_channel channel;
    std::vector<uint8_t> data;
  };
  std::thread m_binarySendWorkerThread;
  std::thread::id m_binarySendWorkerId;
  std::condition_variable m_binaryCondition;
  std::mutex m_binaryControlMutex;
  std::mutex m_binaryCallbackMutex;
  std::deque<raw_binary_frame> m_binaryFrames;
  std::map<std::pair<binary_channel, uint16_t>,
           std::deque<std::chrono::steady_clock::time_point>> m_binaryRawPending;
  bool m_binaryRawActive = false;
  bool m_binaryPollPending = false;
  bool m_binaryPollSawEvent = false;
  std::chrono::steady_clock::time_point m_binaryPollDeadline;
  bool m_binaryControlPending = false;
  bool m_binaryControlComplete = false;
  uint16_t m_binaryControlCode = 0;
  binary_channel m_binaryControlChannel = binary_channel::send;
  std::vector<uint8_t> m_binaryControlReply;

  // ------------------------------------------------------------------------
  //                                 TLS / SSL
  // ------------------------------------------------------------------------

  /// Enable TLS/SSL
  bool m_bTLS;

  /// How TLS should be selected for connections
  tls_mode m_tlsMode;

  /*!
      the server certificate will be verified and the connection
      aborted if the verification fails.
  */
  bool m_bVerifyPeer;

  /// CA file
  std::string m_cafile;

  /// Path to CA file (can hold filename also)
  std::string m_capath;

  /// Path to CERT file
  std::string m_certfile;

  /// Key file
  std::string m_keyfile;

  /// Password keyfile
  std::string m_pwKeyfile;

  /*!
    Authenticated encryption of binary mode (JSON "binary-encryption",
    "binary-key", "binary-hmac-key"). Applied to both connections on
    connect. See VscpRemoteTcpIf::setBinaryAuthentication.
  */
  bool m_bBinaryAuth           = false;
  uint8_t m_binaryEncryption   = 0;
  uint8_t m_binaryKey[32]      = {};
  uint8_t m_binaryKeyLength    = 0;
  uint8_t m_binaryHmacKey[32]  = {};
};

#endif
