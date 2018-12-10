# C++11 single .h asymmetric coroutine implementation

### API

in namespace coroutine:        
* routine_t create(std::function<void()> f);
* void destroy(routine_t id);
* int resume(routine_t id);
* void yield();
* TYPE await(TYPE(*f)());
* routine_t current();
* class Channel<T> with push()/pop();

### OS

* Linux
* macOS
* Windows

### Demo
						
```cpp
#include "coroutine.h"
#include <iostream>
#include <chrono>

coroutine::Channel<int> channel;

string async_func()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
	return "22";
}

void routine_func1()
{
	int i = channel.pop();
	std::cout << i << std::endl;
	
	i = channel.pop();
	std::cout << i << std::endl;
}

void routine_func2(int i)
{
	std::cout << "20" << std::endl;
	coroutine::yield();
	
	std::cout << "21" << std::endl;

	//run function async
	//yield current routine if result not returned
	string str = coroutine::await(async_func);
	std::cout << str << std::endl;
}

void thread_func()
{
	//create routine with callback like std::function<void()>
	coroutine::routine_t rt1 = coroutine::create(routine_func1);
	coroutine::routine_t rt2 = coroutine::create(std::bind(routine_func2, 2));
	
	std::cout << "00" << std::endl;	
	coroutine::resume(rt1);

	std::cout << "01" << std::endl;
	coroutine::resume(rt2);
	
	std::cout << "02" << std::endl;
	channel.push(10);
	
	std::cout << "03" << std::endl;
	coroutine::resume(rt2);
	
	std::cout << "04" << std::endl;
	channel.push(11);
	
	std::cout << "05" << std::endl;
	
	std::this_thread::sleep_for(std::chrono::milliseconds(6000));
	coroutine::resume(rt2);

	//destroy routine, free resouce allocated
	//Warning: don't destroy routine by itself
	coroutine::destroy(rt1);
	coroutine::destroy(rt2);
}

int main()
{
	std::thread t1(thread_func);
	std::thread t2([](){
		//unsupport coordinating routine crossing threads
	});
	t1.join();
	t2.join();
	return 0;
}
```