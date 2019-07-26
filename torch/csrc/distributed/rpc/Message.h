#pragma once

#include <torch/csrc/distributed/rpc/rpc_headers.h>
#include <torch/serialize.h>
#include <vector>

namespace torch {
namespace distributed {
namespace rpc {

enum MessageType {
  BUILTIN_OP = 0,
  BUILTIN_RET,
  PYTHON_UDF_OP,
  PYTHON_UDF_RET,
  SHUTDOWN,
  UNKNOWN
};

// A message to be sent/received by an RpcAgent.
//
// A Message object contains 4 fields:
//    meta (std::vector<char>): a binary chunk of data.
//    tensors (std::vector<torch::Tensor>): all tensors. Tensor data are not
//        included in the meta, and it is up to the RpcAgent implementation to
//        determine how to serialize them. This design is helpful for
//        communicating super large tensors where serializing all the data at
//        once leads to excessively large memory footprint. An implementation
//        can then serialize and send tensors chunck-by-chunk, in the streaming
//        fashion.
//    type (MessageType): type of the message.
//    id (int64_t): message id, this is used by ProcessGroupAgent to match
//                  request and response. Other implementation can ignore it
//                  if they have their own ways to do matching.
//
// Layers above ``RpcAgent`` only converts BuiltinOp, BuiltinRet, PythonUdfOp,
// and PythonUdfRet into a Message, and it is up to the RpcAgent
// implementation to determine how to serialize a message.
class TORCH_API Message final {
 public:

  Message();

  Message(std::vector<char>&& meta,
          std::vector<torch::Tensor>&& tensors,
          MessageType type);

  Message(std::vector<char>&& meta,
          std::vector<torch::Tensor>&& tensors,
          MessageType type,
          int64_t id);

  Message(const Message& other);
  Message(Message&& other) noexcept;
  Message& operator=(Message const& rhs) &;
  Message& operator=(Message&& rhs) &;
  void swap(Message& rhs) noexcept;
  ~Message();

  const std::vector<char>& meta() const;
  const std::vector<torch::Tensor>& tensors() const;
  const MessageType& type() const;

  bool isRequest() const;
  bool isResponse() const;
  bool isShutdown() const;

  // id is an optional field to match request/response. If an RpcAgent
  // implementation is able to do the matching without using this id, it can be
  // dropped during message serialization.
  int64_t id() const;
  void setId(int64_t id);

 private:
  std::vector<char> meta_;
  std::vector<torch::Tensor> tensors_;
  MessageType type_ = MessageType::UNKNOWN;
  int64_t id_ = -1;
};

} // rpc
} // distributed
} // torch
