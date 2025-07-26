#ifndef SRC_APP_ERROR_H
#define SRC_APP_ERROR_H

#include <expected>
#include <iostream>
#include <string>

namespace common {

enum class ErrorCode {
  Ok = 0,
  NotFound,
  PermissionDenied,
  InvalidArgument,
  InternalError,
  IOError
};

struct AppError {
  ErrorCode code;
  std::string message;

  friend std::ostream &operator<<(std::ostream &os, const AppError &err) {
    return os << "Error (code " << static_cast<int>(err.code)
              << "): " << err.message;
  }
};

using Status = std::expected<void, AppError>;

} // namespace common

#endif
