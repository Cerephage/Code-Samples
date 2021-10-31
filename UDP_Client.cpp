/******************************************************************************
*
* User Datatagram Protocol (UDP) Client
*
******************************************************************************/

#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>

int main(int argc, char* argv[])
{
  // 1) Initialize Winsock

  WSADATA wsaData;
  int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
  if (res != 0)
  {
    std::cout 
      << "Error in WSAStartup: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  // 2) Create a UDP socket

  SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
  if (udpSocket == INVALID_SOCKET)
  {
    std::cout 
      << "Error - the socket was not successfully created: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  // 3) Create a remote address structure from the IP string "35.85.114.0" 
  //and port 8888

  sockaddr_in myAddr;
  memset(&myAddr, 0, sizeof(myAddr));
  myAddr.sin_family = AF_INET;
  myAddr.sin_port = htons(8888);

  res = inet_pton(AF_INET, "35.85.114.0", &myAddr.sin_addr);
  if (res == 0)
  {
    std::cout 
      << "Error - the input is not a valid IPv4 dotted-decimal string: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }
  if (res == -1)
  {
    std::cout 
      << "Error - AF argument is unknown: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  // 4) Send data over the socket

  int len = static_cast<int>(std::strlen(argv[0]));
  int tolen = static_cast<int>(sizeof(sockaddr_in));

  res = sendto(udpSocket
    , argv[0]
    , len
    , 0
    , (const sockaddr*) &myAddr
    , tolen);
  if (res == SOCKET_ERROR)
  {
    std::cout 
      << "Error in sending data over socket: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  // 5) Listen for a response

  char buf[1500];
  memset(&buf, 0, sizeof(buf));
  int fromLength = static_cast<int>(sizeof(myAddr));

  res = recvfrom(udpSocket
    , buf
    , 1500
    , 0
    , (sockaddr*) &myAddr
    , &fromLength);
  if (res == SOCKET_ERROR)
  {
    std::cout 
      << "Error in receiving response: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  std::cout << buf << std::endl;

  // 6) Close the socket

  res = closesocket(udpSocket);
  if (res == SOCKET_ERROR)
  {
    std::cout 
      << "Error in closing socket: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }

  // 7) Shut down Winsock

  res = WSACleanup();
  if (res == SOCKET_ERROR)
  {
    std::cout 
      << "Error in WSACleanup: " 
      << WSAGetLastError() 
      << std::endl;
    return 1;
  }
}