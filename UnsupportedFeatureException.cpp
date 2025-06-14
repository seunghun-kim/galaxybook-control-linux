#include "UnsupportedFeatureException.h"

UnsupportedFeatureException::UnsupportedFeatureException(const QString& message)
    : std::runtime_error(message.toStdString())
{
}

UnsupportedFeatureException::UnsupportedFeatureException(const char* message)
    : std::runtime_error(message)
{
} 