#ifndef RR_EDITOR_UI_MODALS_ERROR_H
#define RR_EDITOR_UI_MODALS_ERROR_H

#include <fmt/core.h>

#include <string>
#include <vector>

namespace rr::editor::ui {

class ErrorModal {
   public:
    void draw();

    template <typename... Args>
    static void push_error(const std::string& title,
                           fmt::format_string<Args...> fmt,
                           Args&&... args) {
        s_Errors.push_back(
            {title, fmt::format(fmt, std::forward<Args>(args)...)});
    }

   private:
    struct Error {
        std::string Title;
        std::string Message;
    };
    static std::vector<Error> s_Errors;
};

}  // namespace rr::editor::ui

#endif  // !RR_EDITOR_UI_MODALS_ERROR_H
