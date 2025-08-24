#include <sol/sol.hpp>
#include "curling.hpp"
#include "repl.hpp"

void register_curling(sol::state& lua) {
    using namespace curling;

    lua.new_enum<Request::Method>("HttpMethod", {
        {"GET", Request::Method::GET},
        {"POST", Request::Method::POST},
        {"PUT", Request::Method::PUT},
        {"DELETE", Request::Method::DEL},
        {"PATCH", Request::Method::PATCH},
        {"HEAD", Request::Method::HEAD},
        {"MIME", Request::Method::MIME}
    });

    lua.new_enum<Request::HttpVersion>("HttpVersion", {
        {"DEFAULT", Request::HttpVersion::DEFAULT},
        {"HTTP_1_1", Request::HttpVersion::HTTP_1_1},
        {"HTTP_2", Request::HttpVersion::HTTP_2},
        {"HTTP_3", Request::HttpVersion::HTTP_3}
    });

    lua.new_usertype<Response>("Response",
        "httpCode", &Response::httpCode,
        "body", &Response::body,
        "headers", &Response::headers,
        "toString", &Response::toString,
        "getHeader", &Response::getHeader
    );

    lua.new_usertype<Request>("Request",
        sol::constructors<Request()>(),

        "setMethod", &Request::setMethod,
        "setURL", &Request::setURL,
        "addHeader", &Request::addHeader,
        "setBody", &Request::setBody,
        "send", &Request::send,
        "reset", &Request::reset,
        "setTimeout", &Request::setTimeout,
        "setConnectTimeout", &Request::setConnectTimeout,
        "setFollowRedirects", &Request::setFollowRedirects,
        "setUserAgent", &Request::setUserAgent,
        "setHttpVersion", &Request::setHttpVersion,
        "addArg", &Request::addArg,
        "setAuthToken", &Request::setAuthToken,
        "downloadToFile", &Request::downloadToFile,
        "setCookiePath", &Request::setCookiePath,
        "addFormField", &Request::addFormField,
        "addFormFile", &Request::addFormFile,
        "enableVerbose", &Request::enableVerbose
    );

    lua["curling_version"] = &curling::version;
    lua["waitMS"] = &curling::waitMs;
}

int main() {
    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table, sol::lib::string, sol::lib::math);

    register_curling(lua);

    repl::REPL shell([&lua](const std::string& input) {
        sol::protected_function_result res = lua.safe_script(input, sol::script_pass_on_error);
        if (!res.valid()) {
            sol::error err = res;
            std::cerr << "Lua error: " << err.what() << '\n';
        }
    });

    shell.run();

    return 0;
}
