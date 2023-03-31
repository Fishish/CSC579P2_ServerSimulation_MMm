#include <iostream>
#include <math.h>
#include <list>
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

        Event(long id, float time, char type){ // construct the event object
            this->id = id;
            this->time = time;
            this->type = type;
        }
        Event(){
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

        Client(long id, float aTime, float dTime){ // construct the client object
            this->id = id;
            this->dTime = dTime;
            this->aTime = aTime;
        }
        Client(long id, float aTime){
            this->id = id;
            this->aTime = aTime;
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
        return id < clientObj.id;
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



/*
    Main function to run the program
    it receives 3 inputs and then display the resutls.
*/ 
int main(){
    std::list<int> clientQueue; // A list to represent the queue.
    clientQueue.push_back(1);
    clientQueue.push_back(2);
    clientQueue.push_back(3);

    for (int temp: clientQueue){
        cout << temp << endl;
    }



}


// • the value of the input parameter λ
// • the value of the input parameter K
// • the value of the master clock at the end of the simulation
// • the average service time (based on the K served customers only) -> time joined the queue - time left the queue
//      previous d time = current d time
// • the average waiting time (based on the K served customers only) -> arrival time - time join the queue
// • the arrival time, service time, time of departure of customers L, L + 1, L + 10, and L + 11, as well as the number of customers in the system immediately after the departure of each of these customers.



// for confidence interval x mean + (s/sqrt(n) * z score)




        for (int r = 0; r < kRuns; r++ ){
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
                            clientQueue.front().dTime = temp.time;
                            clientQueue.front().setWaitTime(clientQueue.front().dTime - clientQueue.front().aTime - clientQueue.front().serviceTime);
                            if (count > transient_period){
                                averageWaitingTime += clientQueue.front().waitingTime;
                                averageServiceTime += clientQueue.front().serviceTime;
                            }
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


                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                cout << averageWaitingTime << endl;

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
                        // cout << t1 << endl;


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
                        // cout << masterClock << endl;
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
                            clientQueue.back().dTime = temp.time;
                            clientQueue.back().setWaitTime(clientQueue.back().dTime - clientQueue.back().aTime - clientQueue.back().serviceTime);
                            if (count > transient_period){
                                averageWaitingTime += clientQueue.back().waitingTime;
                                averageServiceTime += clientQueue.back().serviceTime;
                                // cout << clientQueue.back().waitingTime << endl;
                                cout << clientQueue.back().aTime << " depart " << clientQueue.back().dTime << " service "  << clientQueue.back().serviceTime << endl;


                            }
                            // create a client object with the id, atime, dtime.
                            // Client tempClient(temp.id, clientQueue.front().time, temp.time);
                            clientQueue.pop_back(); // remove it from queue
                            if(count == k + transient_period){ // stop if k cutomer is served
                                // cout << eventList.front().type << " " << eventList.front().time << " " << eventList.front().id <<endl;
                                // cout << count << endl;
                                break;
                            }
                            if (clientQueue.size() != 0){ // when queue is not empty

                                clientQueue.back().setDtime(clientQueue.back().serviceTime + masterClock); // set the depart time for next client
                                eventList.push_back(Event(clientQueue.back().id, clientQueue.back().dTime, 'D'));
                                eventList.sort(); // sort the list
                                // cout << clientQueue.front().dTime << endl;
                                // cout << masterClock << endl;
                                // cout << masterClock << endl;
                                // cout << clientQueue.back().aTime << " depart " << clientQueue.back().dTime << " service "  << clientQueue.back().serviceTime << endl;

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
                // cout << averageWaitingTime << endl;


                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                // cout << averageWaitingTime << endl;

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
    












    for (int r = 0; r < kRuns; r++ ){
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
                            clientQueue.front().setWaitTime(clientQueue.front().dTime - clientQueue.front().aTime - clientQueue.front().serviceTime);
                            if (count > transient_period){
                                averageWaitingTime += clientQueue.front().waitingTime;
                                averageServiceTime += clientQueue.front().serviceTime;
                            }
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


                averageSystemTime = (averageServiceTime + averageWaitingTime) / k;
                averageServiceTime = averageServiceTime/k;  // total service time divided by k
                averageWaitingTime = averageWaitingTime/k;  // total waiting time divided by k
                // serviceMeanList.push_back(averageServiceTime);
                waitingMeanList.push_back(averageWaitingTime);
                systemMeanList.push_back(averageSystemTime);
                totalSystemTime +=  averageSystemTime;
                // totalAverageServiceTime += averageServiceTime;
                totalAverageWaitingTime += averageWaitingTime;
                // cout << averageWaitingTime << endl;

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
    
        