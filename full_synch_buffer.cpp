#include <iostream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <unistd.h>

template <typename Type_>
struct buffer {
    virtual ~buffer() {}

    virtual void put(Type_&) = 0;

    virtual Type_ get(void) = 0;
};

using namespace std;

template <typename Type_>
class full_synch_buffer : public buffer<unique_ptr<Type_>> {
public:
    full_synch_buffer()
        : element_{nullptr}
        , mtx_{}
        , cond_{}
    {
    }

    void put(unique_ptr<Type_>& element) override {
        {
            unique_lock<mutex> lock__(mtx_);
            cond_.wait(lock__, [&]{return element_==nullptr;});
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
        cond_.notify_one();
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
	explicit Consumer(full_synch_buffer<Element>& fsb) : fsb_(fsb){}
	full_synch_buffer<Element>& fsb_;
	 void operator()() {
		
		while(true){
			usleep(500000);
			cout<<"Consumer"<<endl;
			unique_ptr<Element> get = fsb_.get();
			
		}
	}
};

class Producer {
public:
	explicit Producer(full_synch_buffer<Element>& fsb) : fsb_(fsb){}
	full_synch_buffer<Element>& fsb_;
	 void operator()() {
		
		int i=0;
		while(true){
			cout<<"Producer"<<endl;
			unique_ptr<Element> put(new Element(i));
			fsb_.put(put);
			i++;
		}
	}
};
int main() {

  	full_synch_buffer<Element> fsb;
	thread ConsumerThread( (Consumer(fsb)) );
	thread ProducerThread( (Producer(fsb)) );
	ProducerThread.join();
	ConsumerThread.join();

	cout<<"hi!"<<endl;

	return 0;
}
