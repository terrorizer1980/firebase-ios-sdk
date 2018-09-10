/*
 * Copyright 2018 Google
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_DATASTORE_H_
#define FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_DATASTORE_H_

#if !defined(__OBJC__)
#error "This header only supports Objective-C++"
#endif  // !defined(__OBJC__)

#include <memory>
#include <string>
#include <vector>

#include "Firestore/core/src/firebase/firestore/auth/credentials_provider.h"
#include "Firestore/core/src/firebase/firestore/core/database_info.h"
#include "Firestore/core/src/firebase/firestore/model/document_key.h"
#include "Firestore/core/src/firebase/firestore/remote/grpc_connection.h"
#include "Firestore/core/src/firebase/firestore/remote/grpc_stream.h"
#include "Firestore/core/src/firebase/firestore/remote/grpc_stream_observer.h"
#include "Firestore/core/src/firebase/firestore/remote/grpc_unary_call.h"
#include "Firestore/core/src/firebase/firestore/remote/remote_objc_bridge.h"
#include "Firestore/core/src/firebase/firestore/util/async_queue.h"
#include "Firestore/core/src/firebase/firestore/util/executor.h"
#include "Firestore/core/src/firebase/firestore/util/status.h"
#include "absl/strings/string_view.h"
#include "grpcpp/completion_queue.h"
#include "grpcpp/support/status.h"

#import <Foundation/Foundation.h>
#import "Firestore/Source/Core/FSTTypes.h"
#import "Firestore/Source/Model/FSTMutation.h"
#import "Firestore/Source/Remote/FSTSerializerBeta.h"

namespace firebase {
namespace firestore {
namespace remote {

class Datastore : public std::enable_shared_from_this<Datastore> {
 public:
  Datastore(const core::DatabaseInfo& database_info,
            util::AsyncQueue* worker_queue,
            auth::CredentialsProvider* credentials,
            FSTSerializerBeta* serializer);

  void Shutdown();

  std::unique_ptr<GrpcStream> CreateGrpcStream(absl::string_view rpc_name,
                                               absl::string_view token,
                                               GrpcStreamObserver* observer);

  void CommitMutations(NSArray<FSTMutation*>* mutations,
                       FSTVoidErrorBlock completion);
  void LookupDocuments(const std::vector<model::DocumentKey>& keys,
                       FSTVoidMaybeDocumentArrayErrorBlock completion);

  static std::string GetWhitelistedHeadersAsString(
      const GrpcStream::MetadataT& headers);

  Datastore(const Datastore& other) = delete;
  Datastore(Datastore&& other) = delete;
  Datastore& operator=(const Datastore& other) = delete;
  Datastore& operator=(Datastore&& other) = delete;

 private:
  void PollGrpcQueue();

  using OnToken = std::function<void(absl::string_view)>;
  using OnError = std::function<void(const util::Status&)>;
  void WithToken(const OnToken& on_token, const OnError& on_error);

  static GrpcStream::MetadataT ExtractWhitelistedHeaders(
      const GrpcStream::MetadataT& headers);
  void LogHeaders(absl::string_view headers, absl::string_view rpc);

  util::AsyncQueue* worker_queue_ = nullptr;
  auth::CredentialsProvider* credentials_ = nullptr;

  // A separate executor dedicated to polling gRPC completion queue (which is
  // shared for all spawned gRPC streams and calls).
  std::unique_ptr<util::internal::Executor> dedicated_executor_;
  grpc::CompletionQueue grpc_queue_;
  GrpcConnection grpc_connection_;

  std::vector<std::unique_ptr<GrpcUnaryCall>> commit_calls_;
  std::vector<std::unique_ptr<GrpcStreamingReader>> lookup_calls_;
  bridge::DatastoreSerializer serializer_bridge_;
};

}  // namespace remote
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_DATASTORE_H_
