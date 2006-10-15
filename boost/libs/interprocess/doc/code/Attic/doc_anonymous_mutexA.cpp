   #include <boost/interprocess/shared_memory_object.hpp>
   #include <boost/interprocess/mapped_region.hpp>
   #include <boost/interprocess/sync/scoped_lock.hpp>
   #include "doc_anonymous_mutex_shared_data.hpp"
   #include <iostream>
   #include <cstdio>

   using namespace boost::interprocess;
   
   int main ()
   {
      try{
         //Erase previous shared memory
         shared_memory_object::remove("shared_memory");

         //Create a shared memory object.
         shared_memory_object shm
            (create_only               //only create
            ,"shared_memory"           //name
            ,read_write   //read-write mode
            );

         //Set size
         shm.truncate(sizeof(shared_memory_log));

         //Map the whole shared memory in this process
         mapped_region region
            (shm                       //What to map
            ,read_write   //Map it as read-write
            );

         //Get the address of the mapped region
         void * addr       = region.get_address();

         //Construct the shared structure in memory
         shared_memory_log * data = new (addr) shared_memory_log;

         //Write some logs
         for(int i = 0; i < shared_memory_log::NumItems; ++i){
            //Lock the mutex
            scoped_lock<interprocess_mutex> lock(data->mutex);
            std::sprintf(data->items[(data->current_line++) % shared_memory_log::NumItems]
                   ,"%s_%d", "process_a", i);
            if(i == (shared_memory_log::NumItems-1))
               data->end_a = true;
            //Mutex is released here
         }
   
         //Wait until the other process ends
         while(1){
            scoped_lock<interprocess_mutex> lock(data->mutex);
            if(data->end_b)
               break;
         }
      }
      catch(interprocess_exception &ex){
         std::cout << ex.what() << std::endl;
         return 1;
      }

      //Erase shared memory
      shared_memory_object::remove("shared_memory");

      return 0;
   }
