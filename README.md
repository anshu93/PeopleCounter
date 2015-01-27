# PeopleCounter
Software to implement an ultrasonic based system to count the number of people in the room.

This project was implemented by me along with Minhazul Islam and Jonathan Victor as part of an Embedded Systems class at Duke in the Fall of 2014.

The project is an arduino based system that uses a mesh of ping sensors to accurately count the number of people in the room and upload this information to the internet where it can be accessed remotely.

The main components used in the project are as follows:

1) 3 x PCBs running ATMEGA 328p
2) 2 x Xbee
3) 1 x Addicore Voice Module
4) 4 x Parallax PING sensors

Please refer to the attached presentation for more information about the system.

# Features

1) Could count the number of people in the room and transfer this information to a centralized hub that would upload this information to the internet.
2) The system would prompt a person to say their name on entry and if it matched a predefined database, the system would log it as such. If there was no match after 2 tries however, the system assumes it is an unauthorized entry and displays that information on the web portal.
