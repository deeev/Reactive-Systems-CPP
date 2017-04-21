#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_) = 0;

    virtual Type_ get(void) = 0;
};

template <typename Type_>
class full_synch_buffer : public buffer<std::unique_ptr<Type_>> {
public:
    full_synch_buffer()
        : element_{nullptr}
        , mtx_{}
        , cond_{}
    {
    }

    void put(std::unique_ptr<Type_> element) override {
        {
            std::unique_lock<std::mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_==nullptr;});
            element_ = std::move(element);
        }
        cond_.notify_one();
    }

    std::unique_ptr<Type_> get() override {
        decltype(element_) return__ = nullptr;
        {
            std::unique_lock<std::mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_!=nullptr;});
            return__ = std::move(element_);
        }
        cond_.notify_one();
        return return__;
    }

private:
    std::unique_ptr<Type_> element_;
    std::mutex mtx_;
    std::condition_variable cond_;
};


typedef int Element;

class Consumer {
public:
	explicit Consumer(full_synch_buffer<Element>& b) : b1(b){}
	full_synch_buffer<Element>& b1;
	 void operator()() {
		
		while(true){
			usleep(500000);
			std::unique_ptr<int> r = b1.get();
           		std::cout << "Get "<< *r <<" from Buffer"<<std::endl;

		}
	}
};

class Producer {
public:
	explicit Producer(full_synch_buffer<Element>& b) : b1(b){}
	full_synch_buffer<Element>& b1;
	 void operator()() {
		
		int i=0;
		while(true){
		
		i++;
		b1.put(std::move(std::unique_ptr<int> (new int(i))));
		std::cout << "Put " <<i<<" into Buffer"<<std::endl;
		}
	}
};
int main() {

  	full_synch_buffer<Element> fsb;
	std::thread ConsumerThread( (Consumer(fsb)) );
	std::thread ProducerThread( (Producer(fsb)) );
	ProducerThread.join();
	ConsumerThread.join();

	

	return 0;
}
