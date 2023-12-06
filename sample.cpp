#include <vector>
#include "pgbar/range.hpp"

int main()
{
    std::size_t TOTAL = 100000000; // The maximum task load without seriously slowing down the program.

    pgbar::pgbar bar; bar.set_task(TOTAL).set_left_bracket(" ").set_right_bracket("").set_todo_char("\033[31m━\033[0m").set_done_char("\033[32m━\033[0m");
    bar.set_style(pgbar::style_opts::entire).set_step(2); // Set the step.

    for (std::size_t i = 0; i<(TOTAL/2); ++i) {
        bar.update(); // Normal update
        // Do anything you want here...
    }

    bar.reset().set_style(pgbar::style_opts::percentage | pgbar::style_opts::task_counter | pgbar::style_opts::countdown);
    std::cout << "Task progress: "; // `range` needs a progress bar to show the progress situation
    for (auto iter : pgbar::range(TOTAL, bar))
        continue; // You can specify a range using `range`
    // The iterator will automatically count the number of tasks

    std::vector<double> arr {};
    for (std::size_t i = 0; i < TOTAL; ++i)
        arr.push_back(i);
    bar.reset().set_style(pgbar::style_opts::entire & ~pgbar::style_opts::bar);
    for (auto iter : pgbar::range(arr, bar))
        continue; // Using a container with elements as the range

    float *pointer_arr = new float[TOTAL] {0.0};
    bar.reset().set_style(pgbar::style_opts::task_counter);
    std::cout << "Pointer arrays is okay: ";
    for (auto iter : pgbar::range(pointer_arr+TOTAL-1, pointer_arr-1, bar))
        continue; // Also can pass a iterator as the range

    delete[] pointer_arr; return 0;
}
