//
//  main.cpp
//  Singaletim
//
//  Created by Atropos on 2016/11/11.
//  Copyright © 2016年 AnaBit. All rights reserved.
//

#include "signalemit.h"


struct Device {
    std::string name;
    std::string ip;
    std::string version;
};

int main(int argc, const char * argv[]) {
    // insert code here...
    
    Signaletim signal;
    signal.regist("hello", [] (Signaletim::Signal signal) {
        std::cout << "dev name = " << Signaletim::conv<Device>(signal).name << std::endl
        << "      ip = " << Signaletim::conv<Device>(signal).ip << std::endl
        << " version = " << Signaletim::conv<Device>(signal).version << std::endl;
    } );
    
    Device dev {"phone", "192.168.51.41", "verison"};
    std::cout << dev.name << std::endl;
    signal.post("hello", std::move(dev));
    
    return 0;
}
