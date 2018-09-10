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

#ifndef FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_WRITE_STREAM_H_
#define FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_WRITE_STREAM_H_

#if !defined(__OBJC__)
#error "This header only supports Objective-C++"
#endif  // !defined(__OBJC__)

#include <memory>
#include <string>

#include "Firestore/core/src/firebase/firestore/remote/datastore.h"
#include "Firestore/core/src/firebase/firestore/remote/remote_objc_bridge.h"
#include "Firestore/core/src/firebase/firestore/remote/stream.h"
#include "Firestore/core/src/firebase/firestore/util/async_queue.h"
#include "Firestore/core/src/firebase/firestore/util/status.h"
#include "absl/strings/string_view.h"
#include "grpcpp/support/byte_buffer.h"

#import <Foundation/Foundation.h>
#import "Firestore/Source/Core/FSTTypes.h"
#import "Firestore/Source/Model/FSTMutation.h"
#import "Firestore/Source/Remote/FSTSerializerBeta.h"

namespace firebase {
namespace firestore {
namespace remote {

class WriteStream : public Stream {
 public:
  WriteStream(util::AsyncQueue* async_queue,
              auth::CredentialsProvider* credentials_provider,
              FSTSerializerBeta* serializer,
              Datastore* datastore,
              id delegate);

  void SetLastStreamToken(NSData* token);
  NSData* GetLastStreamToken() const;

  void WriteHandshake();
  void WriteMutations(NSArray<FSTMutation*>* mutations);

  bool IsHandshakeComplete() const {
    return is_handshake_complete_;
  }
  // FIXME exists for tests
  void SetHandshakeComplete() {
    is_handshake_complete_ = true;
  }

 private:
  std::unique_ptr<GrpcStream> CreateGrpcStream(
      Datastore* datastore, const absl::string_view token) override;
  void FinishGrpcStream(GrpcStream* call) override;
  void DoOnStreamStart() override;
  util::Status DoOnStreamRead(const grpc::ByteBuffer& message) override;
  void DoOnStreamFinish(const util::Status& status) override;

  std::string GetDebugName() const override { return "WriteStream"; }

  bridge::WriteStreamSerializer serializer_bridge_;
  bridge::WriteStreamDelegate delegate_bridge_;
  bool is_handshake_complete_ = false;
  std::string last_stream_token_;
};

}  // namespace remote
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_CORE_SRC_FIREBASE_FIRESTORE_REMOTE_WRITE_STREAM_H_
