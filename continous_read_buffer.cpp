#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>
#include <type_traits>

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_) = 0;

    virtual Type_ get(void) = 0;
};

template <typename Type_>
class continuous_read_buffer : public buffer<Type_> {
public:
    explicit continuous_read_buffer(const Type_& default_val)
        : element_{default_val}
        , mtx_{}
    {
    }

    void put(Type_ element) override {
        std::lock_guard<std::mutex> lock__(mtx_);
        element_ = element;
    }

    Type_ get(void) override {
        std::lock_guard<std::mutex> lock__(mtx_);
        return element_;
    }

private:
    Type_ element_;
    std::mutex mtx_;
};

typedef int Element;

class Consumer {
public:
	explicit Consumer(continuous_read_buffer<Element>& b) : b1(b){}
	continuous_read_buffer<Element>& b1;
	 void operator()() {
		
		while(true){
			usleep(250000);
			int r = b1.get();
           		std::cout << "Get "<< r <<" from Buffer"<<std::endl;

		}
	}
};

class Producer {
public:
	explicit Producer(continuous_read_buffer<Element>& b) : b1(b){}
	continuous_read_buffer<Element>& b1;
	 void operator()() {
		int i = 0;
		while(true){
			usleep(500000);
		i++;
		b1.put(i);
		std::cout << "Put " <<i<<" into Buffer"<<std::endl;
		}
	}
};
int main() {

  	continuous_read_buffer<Element> crb(0);
	std::thread ConsumerThread( (Consumer(crb)) );
	std::thread ProducerThread( (Producer(crb)) );
	ProducerThread.join();
	ConsumerThread.join();

	

	return 0;
}
