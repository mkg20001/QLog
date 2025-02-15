#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(logGraphics)
Q_DECLARE_LOGGING_CATEGORY(logPlugin)

#define MODULE_IDENTIFICATION(m) static const char *mod_name = m; \
                                 static const QLoggingCategory function_parameters(m".function.parameters"); \
                                 static const QLoggingCategory runtime(m".runtime"); \

#define FCT_IDENTIFICATION QString logging_cat(mod_name); logging_cat.append(".function.entered"); \
                           qCDebug(QLoggingCategory(logging_cat.toLatin1().constData()))<<"***"

typedef enum debug_level
{
    LEVEL_DEBUG_MAX,
    LEVEL_DEBUG_FUNCTION_PARAMETERS,    LEVEL_DEBUG_FUNCTION_CALLS,
    LEVEL_DEBUG_RUNTIME,
    LEVEL_PRODUCTION
} DEBUG_LEVEL_TYPE;

void set_debug_level(DEBUG_LEVEL_TYPE);

#endif // DEBUG_LOG_H
