#include "async_queue.hpp"


void blah(int &c, int a, int b) {
    std::cout << "starting" << std::endl;
    for(int i = 0; i < b; i++)
        c += a;// + b;
    std::cout << "ending" << std::endl;
}

int main()
{
    std::cout << "test" << std::endl;
    async_queue my_queue;
    int c = 0;
    my_queue.enqueue(blah, std::ref(c), 1, 500);
    my_queue.sync();
    std::cout << c << std::endl;
    my_queue.enqueue(blah, std::ref(c), 7, 5000);
    my_queue.enqueue(blah, std::ref(c), 9, 5000000000);
    my_queue.enqueue(blah, std::ref(c), 1, 15);
    for(int i = 0; i < 100; i++)
        std::cout << c << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::cout << c << std::endl;
    return 0;
}
