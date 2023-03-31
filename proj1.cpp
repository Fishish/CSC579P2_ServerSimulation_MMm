#include <iostream>
#include <math.h>
#include <list>
#include <chrono>

using std::cout;
using std::endl;
using std::cin;
#define IA 16807
#define IM 2147483647
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define MASK 123459876 /* unlikely value*/
#define transient_period 1000 // observations to ignore

// class for the event object
class Event{
    public:
        long id; // the id of event
        float time; // the time of the even
        char type; // the type of event A for arrival and D for depart
        float serviceTime;

        Event(long id, float time, char type){ // construct the event object
            this->id = id;
            this->time = time;
            this->type = type;
            this->serviceTime = -1.0;
        }
        Event(){
        }

        void setServiceTime(float st){
            this->serviceTime = st;
        }

    // comparator for sorting, small time goes to front of the list.
    bool operator <(const Event & eventObj) const
    {
        return time < eventObj.time;
    }
};

// Object to represent each single client for analysis.
class Client{
    public:
        long id; // the id of client
        float dTime; // the depart time of client
        float aTime; // the arrival time of client
        float serviceTime; // the service time of the client
        float waitingTime; // the waiting time of the client.
        int queueNum;
        Client(long id, float aTime, float dTime){ // construct the client object
            this->id = id;
            this->dTime = dTime;
            this->aTime = aTime;
        }
        Client(long id, float aTime){
            this->id = id;
            this->aTime = aTime;
        }

        void setServiceTime(float sTime){
            this->serviceTime = sTime;
        }

        void setQueueNumber(int qb){
            this->queueNum = qb;
        }
        void setWaitTime(float wTime){
            this->waitingTime = wTime;
        }

        void setDtime(float dtime){
            this->dTime = dtime;
        }

        void setSWTime(float serviceTime, float waitingTime){ // set the service and waiting time
            this->serviceTime = serviceTime;
            this->waitingTime = waitingTime;
        }

    // comparator to make smaller ids to the front of the list.
    bool operator <(const Client & clientObj) const
    {
        return serviceTime > clientObj.serviceTime;
    }
};

//Given method from class.
float ran0(long *idum){
    long k;
    float ans;

    *idum ^= MASK;
    k = (*idum)/IQ;
    *idum = IA*(*idum-k*IQ)-IR*k;
    if (*idum < 0) *idum+=IM;
    ans = AM*(*idum);

    *idum ^= MASK;
    return ans;
}
// Given method from class
float expdev(long *idum, double lamb){
    float ran0(long *idum);
    float dummy;

    do
        dummy = ran0(idum);
    while (dummy == 0.0);
    return -log(dummy) * (1/lamb);
    
}



float getVariance(std::list<float> meanList, float sampleMean, float runs){
    float s = 0;
    for ( int i = 0; i < meanList.size(); i++){
        s += pow((meanList.front() - sampleMean), 2);
        meanList.pop_front();
    }
    s = ( 1/(runs-1) ) * s;
    return s;
}

// 0 for -
// 1 for +
float get_Confidence_Interval(float sampleMean, float sampleVariance, float runs, int sign){
    float kScore = 1.96;
    float ans;
    if (sign == 0){  // 1
        ans =  sampleMean - ( (sqrt(sampleVariance) / sqrt(runs)) * kScore );
    }else{
        ans = sampleMean + ( (sqrt(sampleVariance) / sqrt(runs)) * kScore );
    };   
    return ans;
}

int push_to_queue_by_rp(Client c, int rp, std::list<Client> *q1, std::list<Client> *q2, std::list<Client> *q3, std::list<Client> *q4){
    if (rp < 25){
        c.setQueueNumber(1);
        q1->push_back(c);
        return 1;
    }else if(rp < 50){
        c.setQueueNumber(2);
        q2->push_back(c);
        return 2;
    }else if(rp < 75){
        c.setQueueNumber(3);
        q3->push_back(c);
        return 3;
    }else{
        c.setQueueNumber(4);
        q4->push_back(c);
        return 4;
    }
    return 0;
}

/*
    Main function to run the program
    it receives 3 inputs and then display the resutls.
*/ 
int main(){
    auto start = std::chrono::high_resolution_clock::now();

    int key = 0; // variable to 
    long idum = 4492; // The seed of random generator.
    long k = 100000; // The k value, how many clients before terminates
    float lambda = 0.25; // THe lambda value.
    int mDiscipline = 1; // 1 – FCFS, 2 – LCFS-NP, 3 – SJF-NP, 4 – Prio-NP, 5 – Prio-P. 0 for debug
    // std::list<float> serviceMeanList; // A list to represent the queue.
    std::list<float> waitingMeanList; // A list to represent the queue.
    std::list<float> systemMeanList; // A list to represent the queue.

    float totalAverageServiceTime = 0; // average time of service
    float totalAverageWaitingTime = 0; // average time of waitting
    float totalSystemTime = 0;
    float sampleSystemMean;
    float sampleWaitMean;
    float sampleSystemVariance;
    float sampleSWaitVariance;
    

    int kRuns = 30;


    if (mDiscipline == 1){  // FCFS
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Event> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        clientQueue.push_back(eventList.front()); // add first event to queue
                        eventList.pop_back(); // pop the first event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, expdev(&idum, 1.0)+masterClock, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            clientQueue.push_back(temp); // add to the queue

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // create a client object with the id, atime, dtime.
                            Client tempClient(temp.id, clientQueue.front().time, temp.time);
                            dataList.push_back(tempClient);
                            clientQueue.pop_front(); // remove it from queue
                            if(count == k + transient_period){ // stop if k cutomer is served
                                break;
                            }
                            if (clientQueue.size() != 0){ // when queue is not empty
                                eventList.push_back(Event(clientQueue.front().id, expdev(&idum, 1.0)+masterClock, 'D'));
                                eventList.sort(); // sort the list
                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.push_back(Event(eventList.front().id, expdev(&idum, 1.0)+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                            }
                            // add next dep event to the event list.


                            count++;


                        }

                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients

                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                    }
                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "sample system mean " << sampleSystemMean << endl;
            cout << "sample system variance " << sampleSystemVariance << endl;
            cout << "sample wait mean " << sampleWaitMean << endl;
            cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    }else if ( mDiscipline == 2){ // LCFS-NP
        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Event> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        clientQueue.push_back(eventList.front()); // add first event to queue
                        eventList.pop_back(); // pop the first event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, expdev(&idum, 1.0)+masterClock, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            clientQueue.push_back(temp); // add to the queue

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // create a client object with the id, atime, dtime.
                            Client tempClient(temp.id, clientQueue.back().time, temp.time);
                            dataList.push_back(tempClient);
                            clientQueue.pop_back(); // remove it from queue
                            if(count == k + transient_period){ // stop if k cutomer is served
                                break;
                            }
                            if (clientQueue.size() != 0){ // when queue is not empty
                                eventList.push_back(Event(clientQueue.back().id, expdev(&idum, 1.0)+masterClock, 'D'));
                                eventList.sort(); // sort the list
                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.push_back(Event(eventList.front().id, expdev(&idum, 1.0)+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                            }
                            // add next dep event to the event list.


                            count++;


                        }

                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);

                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients

                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                    }
                    

                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "sample system mean " << sampleSystemMean << endl;
            cout << "sample system variance " << sampleSystemVariance << endl;
            cout << "sample wait mean " << sampleWaitMean << endl;
            cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    


    }else if (mDiscipline == 3){  // SJF-NP
    // roll the arrive event and add to the queue
    // roll the service time and sort the queue based on that
    // based on the servive time and queue generate the next depart event.
    // depart and dequeue.


        for (int r = 0; r < kRuns; r++ ){
                idum += 1;
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Client> clientQueue; // A list to represent the queue.
                float averageServiceTime = 0; // average time of service
                float averageWaitingTime = 0; // average time of waitting
                float averageSystemTime = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        clientQueue.push_back(tempC); // add first client to queue
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            Client tempC(temp.id, temp.time); // ID and arrival time

                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.

                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                            }

                            clientQueue.push_back(tempC); // add to the queue

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            clientQueue.sort();
                            // cout << clientQueue.front().serviceTime <<  " first/last " << clientQueue.back().serviceTime <<endl;
                            clientQueue.front().dTime = temp.time;
                            // create a client object with the id, atime, dtime.
                            // Client tempClient(temp.id, clientQueue.front().time, temp.time);
                            Client tempClient = clientQueue.front();
                            // cout << clientQueue.front().dTime <<  " time " << temp.type <<  " time " <<temp.time <<endl;
                            // cout << tempClient.dTime << " and " << masterClock << " and "<< tempClient.aTime << endl;
                            // cout << temp.type << " or " << temp.time << endl;
                            dataList.push_back(tempClient);
                            clientQueue.pop_front(); // remove it from queue
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }
                            if (clientQueue.size() != 0){ // when queue is not empty
                                clientQueue.front().setDtime(clientQueue.front().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue.front().id, clientQueue.front().dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;


                            }else{  // when queue is empty, push the depart event based on the current arrival
                                eventList.front().setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                                // cout << eventList.front().serviceTime+eventList.front().time << endl;

                            }
                            // add next dep event to the event list.


                            count++;


                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);
                // cout << masterClock << endl;
            
                // Go thorugh the client objects to calculate service and waiting time.
                for (int i = 1; i <= k; i++){
                    std::list<Client>::iterator it = std::next(dataList.end(), -i);
     
                    std::list<Client>::iterator itPrevious = std::next(dataList.end(), -i-1);
                    // normal clients
                    if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;

                        it->setSWTime((it->dTime - it->aTime), 0);
                    }else{ // previous one Departs after current arrive
                        // wait time = previous d time - current arrive time.
                        it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
                        // cout << itPrevious->dTime << " and " << it ->aTime << endl;
                        // cout << it->waitingTime << endl;

                    }
                    averageServiceTime += it->serviceTime;  // add the single service time to get the total time
                    averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

                    // cout << it->waitingTime << endl;
                    

                }

                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;

                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "sample system mean " << sampleSystemMean << endl;
            cout << "sample system variance " << sampleSystemVariance << endl;
            cout << "sample wait mean " << sampleWaitMean << endl;
            cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    

    }else if (mDiscipline == 4){ // 4 – Prio-NP
        int rp = 0;  // possibility for queue.

        for (int r = 0; r < kRuns; r++ ){
                // serviceMeanList = std::list<float>();
                float masterClock = 0.0; // The value of master clock.
                long count = 1; // Counting how many loops for testing.
                Event temp; // temporary event object when getting the new event.
                std::list<Client> dataList;  // List for storing client objects for analyzing
                std::list<Event> eventList; // A list to store current events.
                std::list<Client> clientQueue1; // A list to represent the queue.
                std::list<Client> clientQueue2; // A list to represent the queue.
                std::list<Client> clientQueue3; // A list to represent the queue.
                std::list<Client> clientQueue4; // A list to represent the queue.
                float averageServiceTime1 = 0; // average time of service
                float averageWaitingTime1 = 0; // average time of waitting
                float averageSystemTime1 = 0;
                float averageServiceTime2 = 0; // average time of service
                float averageWaitingTime2 = 0; // average time of waitting
                float averageSystemTime2 = 0;
                float averageServiceTime3 = 0; // average time of service
                float averageWaitingTime3 = 0; // average time of waitting
                float averageSystemTime3 = 0;
                float averageServiceTime4 = 0; // average time of service
                float averageWaitingTime4 = 0; // average time of waitting
                float averageSystemTime4 = 0;
                float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

                long aK = 1; // Current id of arrival event.
                long dK = 1; // current id of departure event.

            
                // start the loop until k customer is served.
                do{
                    if(masterClock==0.0){   // Initilize and add the first event
                        Event e(aK, t1, 'A');
                        eventList.push_back(e);
                        masterClock = t1;

                    }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
                        Client tempC(aK, eventList.front().time); // ID and arrival time
                        tempC.setServiceTime(expdev(&idum, 1.0)); // roll a service time
                        tempC.setDtime(tempC.aTime + tempC.serviceTime); // set the depart time since it is the first one
                        // randomly push to one queue.
                        push_to_queue_by_rp(tempC, int(expdev(&idum, 1.0) * 100000) % 100, &clientQueue1, &clientQueue2, &clientQueue3, &clientQueue4);
                        
                        // cout << int(expdev(&idum, 1.0) * 100000) % 100 << "random "<< endl;
                        // if(ret == 1){
                        //     cout << clientQueue1.back().id << "1"<< endl;
                        // }else if(ret == 2){
                        //     cout << clientQueue2.back().id << "2"<<  endl;
                        // }else if (ret == 3){
                        //     cout << clientQueue3.back().id << "3"<<  endl;
                        // }else{
                        //     cout << clientQueue4.back().id << "4"<<  endl;
                        // }
                        eventList.pop_back(); // pop the first arrival event from event list.
                        // create a new arrival event 
                        // create the depature event for first arrival.
                        // Add them to the event list
                        aK++;
                        eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
                        eventList.push_back(Event(1, tempC.dTime, 'D'));
                        // sort the event list based on the time.
                        eventList.sort();

                    }else{ // process the next event from event case in normal case.
                        // store the current event to temp.
                        temp = eventList.front();
                        // pop the next event
                        eventList.pop_front();
                        // update master clock.
                        masterClock = temp.time;
                        // check the event type
                        if(temp.type == 'A'){ // arrival
                            // put client into the list with id and atime;

                            aK++;
                            Client tempC(temp.id, temp.time); // ID and arrival time

                            if(temp.serviceTime != -1.0){
                                tempC.setServiceTime(temp.serviceTime); // set service time if it is preseted in the event.
                            }else{
                                tempC.setServiceTime(expdev(&idum, 1.0)); // set service time
                            }

                            push_to_queue_by_rp(tempC, int(expdev(&idum, 1.0) * 100000) % 100, &clientQueue1, &clientQueue2, &clientQueue3, &clientQueue4);
                            // clientQueue.push_back(tempC); // add to the queue

                            eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
                            eventList.sort(); // sort the list
                        

                        }else{ // depart
                            // cout << clientQueue.front().serviceTime <<  " first/last " << clientQueue.back().serviceTime <<endl;
                            // check is each queue empty
                            if(clientQueue1.size() != 0 ){    // if queue 1 not empty, pop one
                                clientQueue1.front().dTime = temp.time;
                                clientQueue1.front().setWaitTime(clientQueue1.front().dTime - clientQueue1.front().aTime - clientQueue1.front().serviceTime);
                                if (count > transient_period){
                                    averageWaitingTime1 += clientQueue1.front().waitingTime;
                                    averageServiceTime1 += clientQueue1.front().serviceTime;
                                }
                                clientQueue1.pop_front();
                            }else if (clientQueue2.size() != 0 ){
                                clientQueue2.front().dTime = temp.time;
                                clientQueue2.front().setWaitTime(clientQueue2.front().dTime - clientQueue2.front().aTime - clientQueue2.front().serviceTime);
                                if (count > transient_period){
                                    averageWaitingTime2 += clientQueue2.front().waitingTime;
                                    averageServiceTime2 += clientQueue2.front().serviceTime;
                                }
                                clientQueue2.pop_front();
                            }else if (clientQueue3.size() != 0 ){
                                clientQueue3.front().dTime = temp.time;
                                clientQueue3.front().setWaitTime(clientQueue3.front().dTime - clientQueue3.front().aTime - clientQueue3.front().serviceTime);
                                if (count > transient_period){
                                    averageWaitingTime3 += clientQueue3.front().waitingTime;
                                    averageServiceTime3 += clientQueue3.front().serviceTime;
                                }
                                clientQueue3.pop_front();
                            }else if (clientQueue4.size() != 0 ){
                                clientQueue4.front().dTime = temp.time;
                                clientQueue4.front().setWaitTime(clientQueue4.front().dTime - clientQueue4.front().aTime - clientQueue4.front().serviceTime);
                                if (count > transient_period){
                                    averageWaitingTime4 += clientQueue4.front().waitingTime;
                                    averageServiceTime4 += clientQueue4.front().serviceTime;
                                }
                                clientQueue4.pop_front();
                            }

                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }

                            // if(clientQueue1.size() != 0 ){    // if queue 1 not empty, pop one

                            // }else if (clientQueue2.size() != 0 ){
   
                            // }else if (clientQueue3.size() != 0 ){
   
                            // }else if (clientQueue4.size() != 0 ){
  
                            // }
                            if(clientQueue1.size() != 0 ){    // if queue 1 not empty, pop one
                                clientQueue1.front().setDtime(clientQueue1.front().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue1.front().id, clientQueue1.front().dTime, 'D'));
                                eventList.sort(); // sort the list
                            }else if (clientQueue2.size() != 0 ){
                                clientQueue2.front().setDtime(clientQueue2.front().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue2.front().id, clientQueue2.front().dTime, 'D'));
                                eventList.sort(); // sort the list
                            }else if (clientQueue3.size() != 0 ){
                                clientQueue3.front().setDtime(clientQueue3.front().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue3.front().id, clientQueue3.front().dTime, 'D'));
                                eventList.sort(); // sort the list
                            }else if (clientQueue4.size() != 0 ){
                                clientQueue4.front().setDtime(clientQueue4.front().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue4.front().id, clientQueue4.front().dTime, 'D'));
                                eventList.sort(); // sort the list
                            }else{
                                // when queue is empty, push the depart event based on the current arrival
                                eventList.front().setServiceTime(expdev(&idum, 1.0)); // preset the service time 
                                eventList.push_back(Event(eventList.front().id, eventList.front().serviceTime+eventList.front().time, 'D'));
                                eventList.sort(); // sort the list 
                            }
                            count++;


                        }
                        // cout << eventList.front().type <<" " << eventList.front().time << endl;
                    } // big loop for add/dep
                }while (1);

                ////////////////////////////////////////////////////////////////////////////////////////////////////
                // averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                // averageServiceTime = averageServiceTime/k;  // total service time divided by k
                // averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // // serviceMeanList.push_back(averageServiceTime);
                // waitingMeanList.push_back(averageWaitingTime);
                // systemMeanList.push_back(averageSystemTime);
                // totalSystemTime +=  averageSystemTime;
                // // totalAverageServiceTime += averageServiceTime;
                // totalAverageWaitingTime += averageWaitingTime;
                // // cout << averageWaitingTime << endl;
                ////////////////////////////////////////////////////////////////////////////////////////////////////
                // Output

                // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
                // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
                // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

            }

            sampleSystemMean = totalSystemTime / float(kRuns);
            sampleWaitMean = totalAverageWaitingTime / float(kRuns);
            sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
            sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

            cout << "Program Output" << endl;
            cout << "\nThe value of the input parameter lambda: " << lambda << endl;
            cout << "The value of the input parameter K: " << k << endl;
            cout << "sample system mean " << sampleSystemMean << endl;
            cout << "sample system variance " << sampleSystemVariance << endl;
            cout << "sample wait mean " << sampleWaitMean << endl;
            cout << "sample wait variance " << sampleSWaitVariance << endl;
            cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
            cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
            cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
            cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
        
    }else if (mDiscipline == 0){
    Event e(1, 1.1, 'A');
    Event e1(2, 2.2, 'D');

    std::list<Event> eventList; // A list to store current events.
    eventList.push_front(e);
    eventList.push_front(e1);
    eventList.sort();
    eventList.push_front(e1);
    eventList.sort();
    cout << eventList.front().time << endl;

    std::list<Client> clientQueue; // A list to represent the queue.
    Client tempC(1, 1.1); // ID and arrival time
    tempC.setServiceTime(1); // roll a service time
    Client tempCC(2, 2.2); // ID and arrival time
    tempCC.setServiceTime(2); // roll a service time

    clientQueue.push_front(tempC);
    clientQueue.push_front(tempCC);

    clientQueue.sort();
    clientQueue.push_front(tempCC);
        clientQueue.sort();

    cout << clientQueue.front().serviceTime << endl;
    }
                            //     for (int r = 0; r < kRuns; r++ ){
    //             // serviceMeanList = std::list<float>();
    //             float masterClock = 0.0; // The value of master clock.
    //             long count = 0; // Counting how many loops for testing.
    //             Event temp; // temporary event object when getting the new event.
    //             std::list<Client> dataList;  // List for storing client objects for analyzing
    //             std::list<Event> eventList; // A list to store current events.
    //             std::list<Event> clientQueue; // A list to represent the queue.
    //             float averageServiceTime = 0; // average time of service
    //             float averageWaitingTime = 0; // average time of waitting
    //             float averageSystemTime = 0;
    //             float t1 = expdev(&idum, lambda); // roll a time for the first arrival event.

    //             long aK = 1; // Current id of arrival event.
    //             long dK = 1; // current id of departure event.

            
    //             // start the loop until k customer is served.
    //             do{
    //                 if(masterClock==0.0){   // Initilize and add the first event
    //                     Event e(aK, t1, 'A');
    //                     eventList.push_back(e);
    //                     masterClock = t1;

    //                 }else if(eventList.front().id == 1 && eventList.front().type == 'A'){   // create departure for first event and next arrival.
    //                     clientQueue.push_back(eventList.front()); // add first event to queue
    //                     eventList.pop_back(); // pop the first event from event list.
    //                     // create a new arrival event 
    //                     // create the depature event for first arrival.
    //                     // Add them to the event list
    //                     aK++;
    //                     eventList.push_back(Event(2, expdev(&idum, lambda)+masterClock, 'A'));
    //                     eventList.push_back(Event(1, expdev(&idum, 1.0)+masterClock, 'D'));
    //                     // sort the event list based on the time.
    //                     eventList.sort();

    //                 }else{ // process the next event from event case in normal case.
    //                     // store the current event to temp.
    //                     temp = eventList.front();
    //                     // pop the next event
    //                     eventList.pop_front();
    //                     // update master clock.
    //                     masterClock = temp.time;
    //                     // check the event type
    //                     if(temp.type == 'A'){ // arrival
    //                         // put client into the list with id and atime;

    //                         aK++;
    //                         clientQueue.push_back(temp); // add to the queue

    //                         eventList.push_back(Event(aK, expdev(&idum, lambda)+masterClock, 'A')); // create next arrival event
    //                         eventList.sort(); // sort the list
                        

    //                     }else{ // depart
    //                         // create a client object with the id, atime, dtime.
    //                         Client tempClient(temp.id, clientQueue.front().time, temp.time);
    //                         dataList.push_back(tempClient);
    //                         clientQueue.pop_front(); // remove it from queue
    //                         if(temp.id == k){ // stop if k cutomer is served
    //                             break;
    //                         }
    //                         if (clientQueue.size() != 0){ // when queue is not empty
    //                             eventList.push_back(Event(clientQueue.front().id, expdev(&idum, 1.0)+masterClock, 'D'));
    //                             eventList.sort(); // sort the list
    //                         }else{  // when queue is empty, push the depart event based on the current arrival
    //                             eventList.push_back(Event(eventList.front().id, expdev(&idum, 1.0)+eventList.front().time, 'D'));
    //                             eventList.sort(); // sort the list 
    //                         }
    //                         // add next dep event to the event list.


    //                         count++;


    //                     }

    //                 } // big loop for add/dep
    //             }while (1);
    //             // cout << masterClock << endl;
            
    //             // Go thorugh the client objects to calculate service and waiting time.
    //             for (int i = 0; i < dataList.size(); i++){
    //                 std::list<Client>::iterator it = std::next(dataList.begin(), i);
    //                                     // cout << "debug11 " << it->id << endl;

    //                 if (it->id == 1){
    //                         // first client services time and 0 waiting time
    //                         it->setSWTime((it->dTime - it->aTime), 0);
    //                 }else{
    //                         // normal clients
    //                         std::list<Client>::iterator itPrevious = std::next(dataList.begin(), i-1);
    //                         if(itPrevious->dTime < it->aTime){ // previous one Departs before arrive, no wait time
    //                             it->setSWTime((it->dTime - it->aTime), 0);
    //                         }else{ // previous one Departs after current arrive
    //                             it->setSWTime((it->dTime - itPrevious->dTime), (itPrevious->dTime - it->aTime));
    //                         }
    //                 }
    //                 averageServiceTime += it->serviceTime;  // add the single service time to get the total time
    //                 averageWaitingTime += it->waitingTime;  // add the single waiting time to get the total time

    //             }

    //             averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
    //             averageServiceTime = averageServiceTime/k;  // total service time divided by k
    //             averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
    //             // serviceMeanList.push_back(averageServiceTime);
    //             waitingMeanList.push_back(averageWaitingTime);
    //             systemMeanList.push_back(averageSystemTime);
    //             totalSystemTime +=  averageSystemTime;
    //             // totalAverageServiceTime += averageServiceTime;
    //             totalAverageWaitingTime += averageWaitingTime;

    //             // Output

    //             // cout << "The value of the master clock at the end of the simulation: " << masterClock << endl;
    //             // cout << "The average service time (based on the K served customers only): " << averageServiceTime << endl;
    //             // cout << "The average waiting time (based on the K served customers only): " << averageWaitingTime << endl;  

    //         }

    //         sampleSystemMean = totalSystemTime / float(kRuns);
    //         sampleWaitMean = totalAverageWaitingTime / float(kRuns);
    //         sampleSystemVariance = getVariance(systemMeanList, sampleSystemMean, float(kRuns));
    //         sampleSWaitVariance = getVariance(waitingMeanList, sampleWaitMean, float(kRuns));

    //         cout << "Program Output" << endl;
    //         cout << "\nThe value of the input parameter lambda: " << lambda << endl;
    //         cout << "The value of the input parameter K: " << k << endl;
    //         cout << "sample system mean " << sampleSystemMean << endl;
    //         cout << "sample system variance " << sampleSystemVariance << endl;
    //         cout << "sample wait mean " << sampleWaitMean << endl;
    //         cout << "sample wait variance " << sampleSWaitVariance << endl;
    //         cout << "The average system time and corresponding confidence intervals (based on the K serviced customers only) " << endl;
    //         cout << get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 0)  << " < " << sampleSystemMean << " < " <<  get_Confidence_Interval(sampleSystemMean, sampleSystemVariance, float(kRuns), 1) << endl;
    //         cout << "The average waiting time and corresponding confidence intervals (based on the K serviced customers only)" << endl;
    //         cout << get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 0)  << " < " << sampleWaitMean << " < " <<  get_Confidence_Interval(sampleWaitMean, sampleSWaitVariance, float(kRuns), 1) << endl;
    
    // }
 

    





    ////////////////////////////// Get input //////////////////////////////
    // // Get the inputs from user
    // cout << "Please input the parameter lambda(0 < lambda < 1.0)." << endl;  // Lambda
    // cin >> lambda;
    // while(cin.fail() || lambda >= 1.0 || lambda <= 0.0) {
    //     std::cout << "0 < lambda < 1.0" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> lambda;
    // }    

    // cout << "Please input the number K(Positive Intger Greater than 2)." << endl;  // K
    // cin >> k;
    // while(cin.fail() || k <= 2) {
    //     std::cout << "K must be a positive integer Greater than 2" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> k;
    // }    

    // cout << "Please input the integer L such that 1<L<K." << endl;  // L
    // cin >> customerL;
    // while(cin.fail() || customerL <= 1 || customerL >= k) {
    //     std::cout << "1<L<K" << std::endl;
    //     std::cin.clear();
    //     std::cin.ignore(256,'\n');
    //     std::cin >> customerL;
    // }  
    ////////////////////////////// Get input //////////////////////////////



  
    auto elapsed = std::chrono::high_resolution_clock::now() - start;
    long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(
        elapsed).count();
// To get the value of duration use the count()
// member function on the duration object
    cout << "Time taken by function: "<< microseconds << endl;
}


// • the value of the input parameter λ
// • the value of the input parameter K
// • the value of the master clock at the end of the simulation
// • the average service time (based on the K served customers only) -> time joined the queue - time left the queue
//      previous d time = current d time
// • the average waiting time (based on the K served customers only) -> arrival time - time join the queue
// • the arrival time, service time, time of departure of customers L, L + 1, L + 10, and L + 11, as well as the number of customers in the system immediately after the departure of each of these customers.



// for confidence interval x mean + (s/sqrt(n) * z score)

// prempt a new arrival q2, check is next dpart event has lower priority(no need to check it self since fcfs), push to the queue, next loop
// next arrival is q1, check priority, and found next is lower, so push it to the q1, and update the last arrival q2's service time(q2 service time - q2's arrival - q1's arrivale)
// push q2 back to q2, update dpart event time(clock + q1 service)

// for depart, just go though all queues, and pick one by order for generating  dpart 