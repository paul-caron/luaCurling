#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <termios.h>
#include <unistd.h>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <cerrno>

namespace repl {

// === Terminal state (shared for raw mode + signal safety) ===
static struct termios g_orig_termios;
static volatile sig_atomic_t g_got_sigint = 0;

void restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &g_orig_termios);
}

// === Signal handler for Ctrl+C (SIGINT) ===
void handle_sigint(int) {
    g_got_sigint = 1;
    std::cout << "\n^C\n";
    std::cout.flush();
}

// === RAII class for raw terminal mode ===
class TerminalRawMode {
public:
    TerminalRawMode() {
        // Save original terminal state
        tcgetattr(STDIN_FILENO, &g_orig_termios);
        atexit(restore_terminal);  // Restore on exit

        struct termios raw = g_orig_termios;
        raw.c_lflag &= ~(ECHO | ICANON);
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    }

    ~TerminalRawMode() {
        restore_terminal();
    }
};

// === REPL ===
class REPL {
public:
    using EvalCallback = std::function<void(const std::string&)>;

    REPL(EvalCallback callback,
         const std::string& prompt = ">>> ",
         const std::string& more_prompt = "... ")
        : callback_(callback), prompt_(prompt), more_prompt_(more_prompt), history_index_(0) {}

    void run() {
        // Set up SIGINT handler
        struct sigaction sa {};
        sa.sa_handler = handle_sigint;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0; // No SA_RESTART — we want read() to be interruptible
        sigaction(SIGINT, &sa, nullptr);

        TerminalRawMode raw;
        std::vector<std::string> buffer_lines;

        while (true) {
            std::string line;
            size_t cursor_pos = 0;
            history_index_ = history_.size();

            std::string current_prompt = buffer_lines.empty() ? prompt_ : more_prompt_;
            std::cout << current_prompt << std::flush;
            line.clear();

            while (true) {
                if (g_got_sigint) {
                    g_got_sigint = 0;
                    buffer_lines.clear();
                    std::cout << current_prompt << std::flush;
                    break;  // restart prompt after ^C
                }

                char c;
                ssize_t n = read(STDIN_FILENO, &c, 1);
                if (n == -1 && errno == EINTR) {
                    continue;  // retry on interrupted read
                } else if (n <= 0) {
                    std::cout << "\n";
                    return; // Ctrl+D or read error
                }

                if (c == 4) { // Ctrl+D
                    std::cout << "\n";
                    return;
                } else if (c == '\n') {
                    std::cout << "\n";
                    if (!line.empty() && line.back() == '\\') {
                        line.pop_back();
                        buffer_lines.push_back(line);
                        break;
                    } else {
                        buffer_lines.push_back(line);
                        std::string full_input = join_lines(buffer_lines);
                        if (!full_input.empty()) {
                            history_.push_back(full_input);
                        }
                        callback_(full_input);
                        buffer_lines.clear();
                        break;
                    }
                } else if (c == 127 || c == 8) { // Backspace
                    if (cursor_pos > 0) {
                        line.erase(cursor_pos - 1, 1);
                        cursor_pos--;
                        redraw_line(current_prompt, line, cursor_pos);
                    }
                } else if (c == '\x1b') { // Arrow keys
                    char seq[2];
                    if (read(STDIN_FILENO, &seq[0], 1) != 1) break;
                    if (read(STDIN_FILENO, &seq[1], 1) != 1) break;
                    if (seq[0] == '[') {
                        switch (seq[1]) {
                            case 'D': // ←
                                if (cursor_pos > 0) {
                                    std::cout << "\x1b[1D" << std::flush;
                                    cursor_pos--;
                                }
                                break;
                            case 'C': // →
                                if (cursor_pos < line.size()) {
                                    std::cout << "\x1b[1C" << std::flush;
                                    cursor_pos++;
                                }
                                break;
                            case 'A': // ↑
                                if (history_index_ > 0) {
                                    history_index_--;
                                    line = history_[history_index_];
                                    cursor_pos = line.size();
                                    redraw_line(current_prompt, line, cursor_pos);
                                }
                                break;
                            case 'B': // ↓
                                if (history_index_ + 1 < history_.size()) {
                                    history_index_++;
                                    line = history_[history_index_];
                                    cursor_pos = line.size();
                                } else {
                                    history_index_ = history_.size();
                                    line.clear();
                                    cursor_pos = 0;
                                }
                                redraw_line(current_prompt, line, cursor_pos);
                                break;
                        }
                    }
                } else if (isprint(static_cast<unsigned char>(c))) {
                    line.insert(cursor_pos, 1, c);
                    cursor_pos++;
                    redraw_line(current_prompt, line, cursor_pos);
                }
            }
        }
    }

private:
    EvalCallback callback_;
    std::string prompt_;
    std::string more_prompt_;
    std::vector<std::string> history_;
    size_t history_index_;

    void redraw_line(const std::string& prompt, const std::string& line, size_t cursor_pos) {
        std::cout << "\r\033[K" << prompt << line << std::flush;
        size_t total_pos = prompt.size() + line.size();
        size_t desired_pos = prompt.size() + cursor_pos;
        if (desired_pos < total_pos) {
            size_t move_left = total_pos - desired_pos;
            std::cout << "\x1b[" << move_left << "D" << std::flush;
        }
    }

    std::string join_lines(const std::vector<std::string>& lines) {
        std::string result;
        for (const auto& l : lines) {
            result += l;
        }
        return result;
    }
};

} // namespace repl
