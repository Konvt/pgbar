#include <vector>
#include "pgbar/range.hpp"
constexpr std::size_t TOTAL = 100000000; // The maximum task load without seriously slowing down the program.
double origin_arr[1000*2] {0.0};

int main()
{
    std::cout << "Testing...\n";
    pgbar::pgbar bar; bar.set_task(TOTAL).set_left_bracket(" ").set_right_bracket("").set_todo_char("\033[31m━\033[0m").set_done_char("\033[32m━\033[0m");
    bar.set_style(pgbar::style_opts::entire).set_step(2); // Set the step.

    for (std::size_t i = 0; i<(TOTAL/2); ++i) {
        bar.update(); // Normal update
        // Do anything you want here...
    }

    bar.reset().set_style(pgbar::style_opts::percentage | pgbar::style_opts::task_counter | pgbar::style_opts::countdown);
    std::cout << "Task progress: "; // `range` needs a progress bar to show the progress situation
    for (auto ele : pgbar::range(TOTAL/2, bar))
        continue; // You can specify a range using `range`
    // The iterator will automatically count the number of tasks

    std::vector<double> arr {};
    for (std::size_t i = 0; i < TOTAL/3; ++i)
        arr.push_back(i);
    bar.reset().set_style(pgbar::style_opts::entire & ~pgbar::style_opts::bar);
    for (auto ele : pgbar::range(arr, bar))
        continue; // Using a container with elements as the range

    float *pointer_arr = new float[TOTAL/4] {0.0};
    bar.reset().set_style(pgbar::style_opts::task_counter);
    std::cout << "Pointer arrays is okay: ";
    for (auto ele : pgbar::range(pointer_arr+TOTAL-1, pointer_arr-1, bar))
        continue; // Also can pass a pointer array as the range
    delete[] pointer_arr; pointer_arr = nullptr;

    bar.reset().set_style(pgbar::style_opts::entire);
    for (auto ele : pgbar::range(origin_arr, bar))
        continue;

    return 0;
}
