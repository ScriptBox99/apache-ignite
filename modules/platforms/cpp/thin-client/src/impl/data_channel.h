/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _IGNITE_IMPL_THIN_DATA_CHANNEL
#define _IGNITE_IMPL_THIN_DATA_CHANNEL

#include <stdint.h>

#include <memory>

#include <ignite/future.h>
#include <ignite/thin/ignite_client_configuration.h>

#include <ignite/common/concurrent.h>
#include <ignite/network/socket_client.h>
#include <ignite/network/async_client_pool.h>

#include <ignite/impl/interop/interop_output_stream.h>
#include <ignite/impl/binary/binary_writer_impl.h>

#include "impl/protocol_version.h"
#include "impl/ignite_node.h"
#include "impl/response_status.h"
#include "impl/channel_state_handler.h"
#include "impl/notification_handler.h"

namespace ignite
{
    namespace impl
    {
        namespace interop
        {
            // Forward declaration.
            class InteropMemory;
        }

        namespace thin
        {
            // Forward declaration.
            class Request;

            // Forward declaration.
            class Response;

            /**
             * Data router.
             *
             * Ensures there is a connection between client and one of the servers
             * and routes data between them.
             */
            class DataChannel
            {
            public:
                /** Version set type. */
                typedef std::set<ProtocolVersion> VersionSet;

                /** Shared pointer to DataBuffer Promise. */
                typedef common::concurrent::SharedPointer<common::Promise<network::DataBuffer> > SP_PromiseDataBuffer;

                /** Response map. */
                typedef std::map< int64_t, SP_PromiseDataBuffer> ResponseMap;

                /** Notification handler map. */
                typedef std::map< int64_t, NotificationHandlerHolder > NotificationHandlerMap;

                /** Version 1.2.0. */
                static const ProtocolVersion VERSION_1_2_0;

                /** Version 1.3.0. */
                static const ProtocolVersion VERSION_1_3_0;

                /** Version 1.4.0. Added: Partition awareness support, IEP-23. */
                static const ProtocolVersion VERSION_1_4_0;

                /** Version 1.5.0. Transaction support. */
                static const ProtocolVersion VERSION_1_5_0;

                /** Version 1.6.0. Expiration Policy Configuration. */
                static const ProtocolVersion VERSION_1_6_0;

                /** Version 1.7.0. Features introduced. */
                static const ProtocolVersion VERSION_1_7_0;

                /** Current version. */
                static const ProtocolVersion VERSION_DEFAULT;

                /**
                 * Constructor.
                 *
                 * @param id Connection ID.
                 * @param addr Address.
                 * @param asyncPool Async pool for connection.
                 * @param cfg Configuration.
                 * @param typeMgr Type manager.
                 * @param stateHandler State handler.
                 */
                DataChannel(uint64_t id,
                    const network::EndPoint& addr,
                    const ignite::network::SP_AsyncClientPool& asyncPool,
                    const ignite::thin::IgniteClientConfiguration& cfg,
                    binary::BinaryTypeManager& typeMgr,
                    ChannelStateHandler& stateHandler);

                /**
                 * Destructor.
                 */
                ~DataChannel();

                /**
                 * Perform handshake.
                 *
                 * @return @c true on success.
                 */
                void StartHandshake();

                /**
                 * Close connection.
                 */
                void Close();

                /**
                 * Synchronously send request message and receive response. Uses provided timeout.
                 *
                 * @param req Request message.
                 * @param rsp Response message.
                 * @param timeout Timeout.
                 * @throw IgniteError on error.
                 */
                void SyncMessage(Request& req, Response& rsp, int32_t timeout);

                /**
                 * Process received message.
                 *
                 * @param msg Message.
                 */
                void ProcessMessage(const network::DataBuffer& msg);

                /**
                 * Register handler for the notification.
                 * @param notId Notification ID.
                 * @param handler Handler.
                 */
                void RegisterNotificationHandler(int64_t notId, const SP_NotificationHandler& handler);

                /**
                 * Get remote node.
                 * @return Node.
                 */
                const IgniteNode& GetNode() const
                {
                    return node;
                }

                /**
                 * Get connection ID.
                 * @return Connection ID.
                 */
                uint64_t GetId() const
                {
                    return id;
                }

                /**
                 * Deserialize message received by this channel.
                 * @tparam T Message type.
                 * @param data Data.
                 * @param msg Message.
                 */
                template<typename T>
                void DeserializeMessage(const network::DataBuffer& data, T& msg)
                {
                    interop::InteropInputStream inStream(data.GetInputStream());

                    // Skipping size (4 bytes) and reqId (8 bytes)
                    inStream.Ignore(12);

                    binary::BinaryReaderImpl reader(&inStream);

                    msg.Read(reader, currentVersion);
                }

                /**
                 * Fail all pending requests.
                 *
                 * @param err Error.
                 */
                void FailPendingRequests(const IgniteError* err);

            private:
                IGNITE_NO_COPY_ASSIGNMENT(DataChannel);

                /**
                 * Generate request ID.
                 *
                 * Atomically generates and returns new Request ID.
                 *
                 * @return Unique Request ID.
                 */
                int64_t GenerateRequestId()
                {
                    return common::concurrent::Atomics::IncrementAndGet64(&reqIdCounter);
                }

                /**
                 * Generate message to send.
                 *
                 * @param req Request to serialize.
                 * @param mem Memory to write request to.
                 * @return Message ID.
                 */
                int64_t GenerateRequestMessage(Request& req, interop::InteropMemory& mem);

                /**
                 * Asynchronously send request message and get a future for the response.
                 *
                 * @param req Request message.
                 * @throw IgniteError on error.
                 */
                Future<network::DataBuffer> AsyncMessage(Request &req);

                /**
                 * Perform handshake request.
                 *
                 * @param propVer Proposed protocol version.
                 * @return @c true on success and @c false otherwise.
                 */
                bool DoHandshake(const ProtocolVersion& propVer);

                /**
                 * Synchronously send handshake request message and receive handshake response. Uses provided timeout.
                 * Does not try to restore connection on fail.
                 *
                 * @param propVer Proposed protocol version.
                 * @return @c true if accepted.
                 * @throw IgniteError on error.
                 */
                bool Handshake(const ProtocolVersion& propVer);

                /**
                 * Handle handshake response.
                 *
                 * @param msg Message.
                 */
                void OnHandshakeResponse(const network::DataBuffer& msg);

                /**
                 * Check if the version is supported.
                 *
                 * @param ver Version.
                 * @return True if the version is supported.
                 */
                static bool IsVersionSupported(const ProtocolVersion& ver);

                /** Set of supported versions. */
                const static VersionSet supportedVersions;

                /** State handler. */
                ChannelStateHandler& stateHandler;

                /** Indicates whether handshake has been performed. */
                bool handshakePerformed;

                /** Connection ID */
                uint64_t id;

                /** Async pool. */
                ignite::network::SP_AsyncClientPool asyncPool;

                /** Remote node data. */
                IgniteNode node;

                /** Configuration. */
                const ignite::thin::IgniteClientConfiguration& config;

                /** Metadata manager. */
                binary::BinaryTypeManager& typeMgr;

                /** Protocol version. */
                ProtocolVersion currentVersion;

                /** Request ID counter. */
                int64_t reqIdCounter;

                /** Response map mutex. */
                common::concurrent::CriticalSection responseMutex;

                /** Responses. */
                ResponseMap responseMap;

                /** Notification handlers mutex. */
                common::concurrent::CriticalSection handlerMutex;

                /** Notification handlers. */
                NotificationHandlerMap handlerMap;
            };

            /** Shared pointer type. */
            typedef common::concurrent::SharedPointer<DataChannel> SP_DataChannel;
        }
    }
}

#endif //_IGNITE_IMPL_THIN_DATA_CHANNEL
