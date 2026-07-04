#include <FreeRTOS_CJJ.h>
#include <SafeQueues.hpp>
#include <LGuards.hpp>
#include <ThreadRTOS.hpp>

struct DataStruct
{
  uint8_t id = 0;
  float value = 0.0;
  DataStruct() = default;
  DataStruct(uint8_t id, float value) : id(id), value(value){}
};

// Thread-safe communication queue with a fixed capacity of 5 slots.
// Utilizes static array buffers under the hood to ensure zero dynamic allocation.
SafeQueue<DataStruct, 5> DataStructQueue;

// Mutex to protect Serial
Mutex serial_mux;

// Producer Task Class: Emulates multiple telemetry streams sending data packets.
class Prod : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  private:
    uint8_t id = 1;
    
  public:
    Prod(uint8_t id) : id(id) {}
    
  protected:
    void Run() override
    {
      for(;;)
      {
        // Generate fake sensor data structure payload
        DataStruct data(this->id, static_cast<float>(random(0, 10000)) / 100.f);
        
        // Attempt to push data into the queue.
        // It blocks for a maximum of 1000 milliseconds if the queue is full.
        // If omitted, the default parameter blocks indefinitely until space frees up.
        if(DataStructQueue.send(data, 1000)) 
        {
          // Lock the serial interface to print the confirmation line
          LockGuard lock(serial_mux);
          Serial.print("[Prod]: Sent data struct id = "); 
          Serial.print(data.id); 
          Serial.print(" value = "); 
          Serial.println(data.value);
        }
        else
        {
          // Queue remained packed even after our 1000ms grace timeout
          LockGuard lock(serial_mux);
          Serial.print("[Prod]: Queue is full ("); 
          Serial.print(DataStructQueue.messages_waiting()); 
          Serial.println(" messages waiting)");
        }
        

        Kernel::delay(random(800, 1200));
      }
    }
};


// Consumer Task Class: Processes the queued data elements.
class Recv : public ThreadRTOS<Kernel::min_stack_bytes(), 1>
{
  protected:
    void Run() override
    {
      for(;;)
      {
        DataStruct data;
        
        // Block indefinitely until a data structure is ready to be fetched
        if(DataStructQueue.receive(data)) 
        {
          {
            // Lock scope to protect the Serial output block
            LockGuard lock(serial_mux);
            Serial.print("[Recv]: Received data struct id = "); 
            Serial.print(data.id); 
            Serial.print(" value = "); 
            Serial.print(data.value);
            Serial.print(" ("); 
            Serial.print(DataStructQueue.spaces_available()); 
            Serial.println(" spaces available)");
          }
          
          // Intentional 2-second delay to artificially slow down the consumer.
          // This allows the queue buffer to overflow and forces the Producer timeout case.
          Kernel::delay(2000); 
        }
      }
    }
};

// Instantiate two independent producers (ID 1 and ID 2) competing for the same queue.
Prod p1(1);
Prod p2(2);

// Instantiate one centralized consumer.
Recv rx;




void setup()
{
  Serial.begin(115200);
  while(!Serial){delay(1);}

  p1();
  p2();
  rx();

  Kernel::start();
}

void loop(){/* Not used */}
