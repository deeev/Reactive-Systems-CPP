#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>
#include <type_traits>

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_&) = 0;

    virtual Type_ get(void) = 0;
};

using namespace std;

template <typename Type_>
class continuous_write_buffer: public buffer<unique_ptr<Type_>> {
public:
    continuous_write_buffer()
        : element_{nullptr}
        , mtx_{}
        , cond_{}
    {
    }

    void put(unique_ptr<Type_>& element) override {
        {
            std::unique_lock<mutex> lock__(mtx_);
            element_ = move(element);
        }
        cond_.notify_one();
    }

    unique_ptr<Type_> get() override {
        decltype(element_) return__ = nullptr;
        {
            unique_lock<mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_!=nullptr;});
            return__ = move(element_);
        }
        return return__;
    }

private:
    unique_ptr<Type_> element_;
    mutex mtx_;
    condition_variable cond_;
};


class Element {
public:
	Element(int i):i_(i){cout<<"Element with value "<< i_ << " created"<<endl;}
	~Element(){cout<<"Element with value "<< i_ << " destroyed"<<endl;}

private:
int i_;
};

class Consumer {
public:
	explicit Consumer(continuous_write_buffer<Element>& cwb) : cwb_(cwb){}
	continuous_write_buffer<Element>& cwb_;
	 void operator()() {
		
		while(true){
			usleep(500000);
			cout<<"Consumer"<<endl;
			unique_ptr<Element> get = cwb_.get();
		}
	}
};

class Producer {
public:
	explicit Producer(continuous_write_buffer<Element>& cwb) : cwb_(cwb){}
	continuous_write_buffer<Element>& cwb_;
	 void operator()() {

		int i = 0;
		while(true){
			usleep(250000);
			cout<<"Producer"<<endl;
			unique_ptr<Element> put(new Element(i));
			cwb_.put(put);
			i++;
		}
	}
};
int main() {

  	continuous_write_buffer<Element> cwb;
	thread ConsumerThread( (Consumer(cwb)) );
	thread ProducerThread( (Producer(cwb)) );
	ProducerThread.join();
	ConsumerThread.join();

	

	return 0;
}
