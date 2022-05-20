#ifndef COLLECTMETRICS_H
#define COLLECTMETRICS_H

#include <iostream>
#include <fstream>
using namespace ns3;

class collectMetrics : public Object {
    public:
        static TypeId GetTypeId (void);
        virtual TypeId GetInstanceTypeId (void) const;
        void addDataCollection (uint64_t packetId, float posX, float posY, double time);
        void openFile (std::string OutputDataFile);
        void closeFile ();
        
    private:
        std::ofstream m_myfile;
	    std::vector<uint64_t> m_packetId;
	    //std::vector<float> m_locationX;
        //std::vector<float> m_locationY;
        //std::vector<double> m_time;
};

TypeId collectMetrics::GetTypeId (void) {
   static TypeId tid = TypeId ("ns3::collectMetrics")
     .SetParent<Object> ();
   return tid;
 }

TypeId collectMetrics::GetInstanceTypeId (void) const {
   return GetTypeId ();
}

void collectMetrics::addDataCollection (uint64_t packetId, float posX, float posY, double time) {
    //m_packetId.push_back(packetId);
    //m_locationX.push_back(posX);
    //m_locationY.push_back(posY);
    //m_time.push_back(time);
    //print values to a file instead
      // will open andclose file, at simulation begin and end
        // avoid mixing data collection and setting up simulation
        //Before simulator run line:  add the pointer for collector before simulation starts, also open file here
        // can pass pointer of files to collector
            // place same collector on all vehices with for loop
        // before finish simulation, after destroy line, can close the file now
        // to keep simple, can keep as one file
    std::cout << "packetID: " << packetId << std::endl;
    std::cout << "posX: " << posX << std::endl;
    std::cout << "posY: " << posY << std::endl;
    std::cout << "time: " << time << std::endl;
    //m_myfile << packetId << "," << posX << "," << posY << "," << time << ",\n";
}

void collectMetrics::openFile (std::string OutputDataFile) {
    m_myfile.open (OutputDataFile);
}

void collectMetrics::closeFile () {
    m_myfile.close();
}
#endif
