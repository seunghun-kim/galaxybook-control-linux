#ifndef UNSUPPORTEDFEATUREEXCEPTION_H
#define UNSUPPORTEDFEATUREEXCEPTION_H

#include <stdexcept>
#include <QString>

class UnsupportedFeatureException : public std::runtime_error
{
public:
    explicit UnsupportedFeatureException(const QString& message);
    explicit UnsupportedFeatureException(const char* message);
};

#endif // UNSUPPORTEDFEATUREEXCEPTION_H 