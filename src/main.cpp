//
//   _______       _       _      _     _            
//  |__   __|     (_)     | |    (_)   | |           
//     | |_      ___ _ __ | |     _  __| | __ _ _ __ 
//     | \ \ /\ / / | '_ \| |    | |/ _` |/ _` | '__|
//     | |\ V  V /| | | | | |____| | (_| | (_| | |   
//     |_| \_/\_/ |_|_| |_|______|_|\__,_|\__,_|_|                                                                            
//
//     Author  : Nadarbreicq, JulesTopart
//     Version : 0.1.0
//     Last update : 10 / 04 / 2023
                                    
#include "TwinLidar.h"

Intercom intercom;
PixelRing pixel;
Lidar lidar;

enum class RingMode{
    LIDAR,
    INTERCOM
} currentMode;

bool lastConnectionState = true;

void parseCommand(String command){
    if(strcmp(command.c_str(), "displayIntercom()") == 0){
        currentMode = RingMode::INTERCOM;
    }else if(strcmp(command.c_str(), "displayLidar()") == 0){
        currentMode = RingMode::LIDAR;
    }else if(command.startsWith("lookAt")){

            String argString = command.substring(command.indexOf("(") +1, command.indexOf(")"));
            String angleStr = argString.substring(0, argString.indexOf(','));
            String distStr  = argString.substring(argString.indexOf(',')+1, argString.length());

            float angle = float(angleStr.toInt());
            float dist = float(distStr.toInt());

            Console::info() << "Angle :" << angle << Console::endl;
            Console::info() << "Dist :" << dist << Console::endl;

            lidar.LookAt(angle, dist);
    }else if(command.startsWith("isSomethingAtAngle")){
            Console::info() << "Command parsed :" << command.c_str() << Console::endl;
            String argString = command.substring(command.indexOf("(") +1, command.indexOf(")"));

            float angle = float(argString.toInt());

            Console::info() << "Angle :" << angle << Console::endl;

            if(lidar.GetDistance(angle) < 300) intercom.SendMessage("STOP");
    }


}

void setup(){
    Console::Initialize();
    Console::SetLevel(ConsoleLevel::_TRACE);

    currentMode = RingMode::INTERCOM;

    pixel.Initialize();
    intercom.Initialize();
    lidar.Initialize();
}


void loop(){
    lidar.Update();
    intercom.Update();
    
    while(intercom.HasPendingCommand()){
        parseCommand(String(intercom.UnstackCommand().c_str()));
    }

    if(currentMode == RingMode::INTERCOM){
        if(lastConnectionState != intercom.IsConnected()){
            lastConnectionState = intercom.IsConnected();
        }
        pixel.DrawIntercom(intercom.IsConnected());
    }else if(currentMode == RingMode::LIDAR){
        pixel.DrawLidar(lidar);
    }

    pixel.Update();
    //delayMicroseconds(10);
}