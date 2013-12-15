//=============================================================================
// Copyright © 2010 NaturalPoint, Inc. All Rights Reserved.
// 
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall NaturalPoint, Inc. or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//=============================================================================

//10.10.67.65
/*

SampleClient.cpp

This program connects to a NatNet server, receives a data stream, and writes that data stream
to an ascii file.  The purpose is to illustrate using the NatNetClient class.

Usage [optional]:

	SampleClient [ServerIP] [LocalIP] [OutputFilename]

	[ServerIP]			IP address of the server (e.g. 192.168.0.107) ( defaults to local machine)
	[OutputFilename]	Name of points file (pts) to write out.  defaults to Client-output.pts


	Trackable 1: Robot front
	Trackable 2: Robot back
	Trackable 3: Kitchen
	Trackable 4: Table 1
	Trackable 5: Table 2
	Trackable 6: Table 3
	Trackable 7: Table 4

*/
#include <math.h>
#include <stdio.h>
#include <tchar.h>
#include <conio.h>
#include <winsock2.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <queue>

#include "NatNetTypes.h"
#include "NatNetClient.h"


// Need to link with Ws2_32.lib, Mswsock.lib, and Advapi32.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")



#pragma warning( disable : 4996 )

void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs);
void _WriteFrame(FILE* fp, sFrameOfMocapData* data);
void _WriteFooter(FILE* fp);
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData);		// receives data from the server
void __cdecl MessageHandler(int msgType, char* msg);		            // receives NatNet error mesages
void resetClient();
int CreateClient(int iConnectionType);
double calculate_angle(sRigidBodyData trackable1, sRigidBodyData trackable2, sRigidBodyData trackable3);
double calculate_distance(sRigidBodyData trackable1, sRigidBodyData trackable2);
unsigned int MyServersDataPort = 3130;
unsigned int MyServersCommandPort = 3131;




NatNetClient* theClient;
FILE* fp;

char szMyIPAddress[128] = "";
char szServerIPAddress[128] = "";
char state = '0';

#define PI 3.14159265358979323846
#define ROBOT_PORT "5000"
#define DEFAULT_BUFLEN 512
#define KITCHEN 2
#define TABLE_ONE 3
#define TABLE_TWO 4
#define TABLE_THREE 5
#define TABLE_FOUR 6


bool init_called = false;
sRigidBodyData location_data[7];
double distances[5][5]; //first number is table number, second is number of destination table
//int destination = KITCHEN;
int dests[11];

struct dest_container {
	int dests[10];
	int step_num;
};
dest_container dest_cont;

int send_cmd(char cmd) {
	    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
                    *ptr = NULL,
                    hints;
    //char sendbuf = 'R';
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    
    // Validate the parameters
    //if (argc != 2) {
    //    printf("usage: %s server-name\n", argv[0]);
    //    return 1;
    //}

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    iResult = getaddrinfo("172.22.11.2", ROBOT_PORT, &hints, &result);//argv[1] 172.22.11.2
    if ( iResult != 0 ) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Attempt to connect to an address until one succeeds
    for(ptr=result; ptr != NULL ;ptr=ptr->ai_next) {

        // Create a SOCKET for connecting to server
        ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, 
            ptr->ai_protocol);
        if (ConnectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

        // Connect to server.
        iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(ConnectSocket);
            ConnectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    // Send an initial buffer
    iResult = send( ConnectSocket, &cmd, 1, 0 );
    if (iResult == SOCKET_ERROR) {
        printf("send failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %ld\n", iResult);

    // shutdown the connection since no more data will be sent
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Receive until the peer closes the connection
    do {

        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if ( iResult > 0 )
            printf("Bytes received: %d\n", iResult);
        else if ( iResult == 0 )
            printf("Connection closed\n");
        else
            printf("recv failed with error: %d\n", WSAGetLastError());

    } while( iResult > 0 );

    // cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}

void init() {
	for (int i = 2; i < 7; i++) {
		for (int j = 2; j < 7; j++) {
			if (i == j) {
				distances[i-2][j-2] = 1000;
			} else {
				distances[i-2][j-2] = calculate_distance(location_data[i], location_data[j]);
			}
		}
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	//send_cmd('R');
	dest_cont.step_num = 0;
	memset(&(dest_cont.dests), 0, 10);
	dest_cont.dests[11] = -1;
	dest_cont.dests[0] = KITCHEN;
	dest_cont.dests[1] = TABLE_ONE;
	dest_cont.dests[2] = TABLE_TWO;
	dest_cont.dests[3] = -1;
    int iResult;
    int iConnectionType = ConnectionType_Multicast;
    //int iConnectionType = ConnectionType_Unicast;
    
    // parse command line args
    if(argc>1)
    {
        strcpy(szServerIPAddress, argv[1]);	// specified on command line
        printf("Connecting to server at %s...\n", szServerIPAddress);
    }
    else
    {
        strcpy(szServerIPAddress, "10.10.67.65");		// not specified - assume server is local machine
        printf("Connecting to server at LocalMachine\n");
    }
    if(argc>2)
    {
        strcpy(szMyIPAddress, argv[2]);	    // specified on command line
        printf("Connecting from %s...\n", szMyIPAddress);
    }
    else
    {
        strcpy(szMyIPAddress, "10.10.66.30");          // not specified - assume server is local machine
        printf("Connecting from LocalMachine...\n");
    }

    // Create NatNet Client
    iResult = CreateClient(iConnectionType);
    if(iResult != ErrorCode_OK)
    {
        printf("Error initializing client.  See log for details.  Exiting");
        return 1;
    }
    else
    {
        printf("Client initialized and ready.\n");
    }


	// send/receive test request
	printf("[SampleClient] Sending Test Request\n");
	void* response;
	int nBytes;
	iResult = theClient->SendMessageAndWait("TestRequest", &response, &nBytes);
	if (iResult == ErrorCode_OK)
	{
		printf("[SampleClient] Received: %s", (char*)response);
	}

	// Retrieve Data Descriptions from server
	printf("\n\n[SampleClient] Requesting Data Descriptions...");
	sDataDescriptions* pDataDefs = NULL;
	int nBodies = theClient->GetDataDescriptions(&pDataDefs);
	if(!pDataDefs)
	{
		printf("[SampleClient] Unable to retrieve Data Descriptions.");
	}
	else
	{
        printf("[SampleClient] Received %d Data Descriptions:\n", pDataDefs->nDataDescriptions );
        for(int i=0; i < pDataDefs->nDataDescriptions; i++)
        {
            printf("Data Description # %d (type=%d)\n", i, pDataDefs->arrDataDescriptions[i].type);
            if(pDataDefs->arrDataDescriptions[i].type == Descriptor_MarkerSet)
            {
                // MarkerSet
                sMarkerSetDescription* pMS = pDataDefs->arrDataDescriptions[i].Data.MarkerSetDescription;
                printf("MarkerSet Name : %s\n", pMS->szName);
                for(int i=0; i < pMS->nMarkers; i++)
                    printf("%s\n", pMS->szMarkerNames[i]);

            }
            else if(pDataDefs->arrDataDescriptions[i].type == Descriptor_RigidBody)
            {
                // RigidBody
                sRigidBodyDescription* pRB = pDataDefs->arrDataDescriptions[i].Data.RigidBodyDescription;
                printf("RigidBody Name : %s\n", pRB->szName);
                printf("RigidBody ID : %d\n", pRB->ID);
                printf("RigidBody Parent ID : %d\n", pRB->parentID);
                printf("Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx, pRB->offsety, pRB->offsetz);
            }
            else if(pDataDefs->arrDataDescriptions[i].type == Descriptor_Skeleton)
            {
                // Skeleton
                sSkeletonDescription* pSK = pDataDefs->arrDataDescriptions[i].Data.SkeletonDescription;
                printf("Skeleton Name : %s\n", pSK->szName);
                printf("Skeleton ID : %d\n", pSK->skeletonID);
                printf("RigidBody (Bone) Count : %d\n", pSK->nRigidBodies);
                for(int j=0; j < pSK->nRigidBodies; j++)
                {
                    sRigidBodyDescription* pRB = &pSK->RigidBodies[j];
                    printf("  RigidBody Name : %s\n", pRB->szName);
                    printf("  RigidBody ID : %d\n", pRB->ID);
                    printf("  RigidBody Parent ID : %d\n", pRB->parentID);
                    printf("  Parent Offset : %3.2f,%3.2f,%3.2f\n", pRB->offsetx, pRB->offsety, pRB->offsetz);
                }
            }
            else
            {
                printf("Unknown data type.");
                // Unknown
            }
        }
	}

	
	// Create data file for writing received stream into
	char szFile[MAX_PATH];
	char szFolder[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szFolder);
	if(argc > 3)
		sprintf(szFile, "%s\\%s", szFolder, argv[3]);
	else
		sprintf(szFile, "%s\\Client-output.pts",szFolder);
	fp = fopen(szFile, "w");
	if(!fp)
	{
		printf("error opening output file %s.  Exiting.", szFile);
		exit(1);
	}
	if(pDataDefs)
		_WriteHeader(fp, pDataDefs);

	// Ready to receive marker stream!
	printf("\nClient is connected to server and listening for data...\n");
	int c;
	bool bExit = false;
	while(c =_getch())
	{
		switch(c)
		{
			case 'q':
				bExit = true;		
				break;	
			case 'r':
				resetClient();
				break;	
            case 'p':
                sServerDescription ServerDescription;
                memset(&ServerDescription, 0, sizeof(ServerDescription));
                theClient->GetServerDescription(&ServerDescription);
                if(!ServerDescription.HostPresent)
                {
                    printf("Unable to connect to server. Host not present. Exiting.");
                    return 1;
                }
                break;	
            case 'f':
                {
                    sFrameOfMocapData* pData = theClient->GetLastFrameOfData();
                    printf("Most Recent Frame: %d", pData->iFrame);
                }
                break;	
            case 'm':	                        // change to multicast
                iResult = CreateClient(ConnectionType_Multicast);
                if(iResult == ErrorCode_OK)
                    printf("Client connection type changed to Multicast.\n\n");
                else
                    printf("Error changing client connection type to Multicast.\n\n");
                break;
            case 'u':	                        // change to unicast
                iResult = CreateClient(ConnectionType_Unicast);
                if(iResult == ErrorCode_OK)
                    printf("Client connection type changed to Unicast.\n\n");
                else
                    printf("Error changing client connection type to Unicast.\n\n");
                break;


			default:
				break;
		}
		if(bExit)
			break;
	}

	// Done - clean up.
	theClient->Uninitialize();
	_WriteFooter(fp);
	fclose(fp);

	return ErrorCode_OK;
}

// Establish a NatNet Client connection
int CreateClient(int iConnectionType)
{
    // release previous server
    if(theClient)
    {
        theClient->Uninitialize();
        delete theClient;
    }

    // create NatNet client
    theClient = new NatNetClient(iConnectionType);

    // [optional] use old multicast group
    //theClient->SetMulticastAddress("224.0.0.1");

    // print version info
    unsigned char ver[4];
    theClient->NatNetVersion(ver);
    printf("NatNet Sample Client (NatNet ver. %d.%d.%d.%d)\n", ver[0], ver[1], ver[2], ver[3]);

    // Set callback handlers
    theClient->SetMessageCallback(MessageHandler);
    theClient->SetVerbosityLevel(Verbosity_Debug);
    theClient->SetDataCallback( DataHandler, theClient );	// this function will receive data from the server

    // Init Client and connect to NatNet server
    // to use NatNet default port assigments
    int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress);
    // to use a different port for commands and/or data:
    //int retCode = theClient->Initialize(szMyIPAddress, szServerIPAddress, MyServersCommandPort, MyServersDataPort);
    if (retCode != ErrorCode_OK)
    {
        printf("Unable to connect to server.  Error code: %d. Exiting", retCode);
        return ErrorCode_Internal;
    }
    else
    {
        // print server info
        sServerDescription ServerDescription;
        memset(&ServerDescription, 0, sizeof(ServerDescription));
        theClient->GetServerDescription(&ServerDescription);
        if(!ServerDescription.HostPresent)
        {
            printf("Unable to connect to server. Host not present. Exiting.");
            return 1;
        }
        printf("[SampleClient] Server application info:\n");
        printf("Application: %s (ver. %d.%d.%d.%d)\n", ServerDescription.szHostApp, ServerDescription.HostAppVersion[0],
            ServerDescription.HostAppVersion[1],ServerDescription.HostAppVersion[2],ServerDescription.HostAppVersion[3]);
        printf("NatNet Version: %d.%d.%d.%d\n", ServerDescription.NatNetVersion[0], ServerDescription.NatNetVersion[1],
            ServerDescription.NatNetVersion[2], ServerDescription.NatNetVersion[3]);
        printf("Client IP:%s\n", szMyIPAddress);
        printf("Server IP:%s\n", szServerIPAddress);
        printf("Server Name:%s\n\n", ServerDescription.szHostComputerName);
    }

    return ErrorCode_OK;

}

// DataHandler receives data from the server
void __cdecl DataHandler(sFrameOfMocapData* data, void* pUserData)
{
	NatNetClient* pClient = (NatNetClient*) pUserData;

	printf("Received frame %d\n", data->iFrame);

	if(fp)
		_WriteFrame(fp,data);
	int i=0;
	
    // same system latency test
    float fThisTick = (float)GetTickCount();
    float fDiff = fThisTick - data->fLatency;
    double dDuration = fDiff;
    printf("Latency (same system) (msecs): %3.2lf\n", dDuration);


	// timecode
	// decode to values
	int hour, minute, second, frame, subframe;
	bool bValid = pClient->DecodeTimecode(data->Timecode, data->TimecodeSubframe, &hour, &minute, &second, &frame, &subframe);
	// decode to friendly string
	char szTimecode[128] = "";
	pClient->TimecodeStringify(data->Timecode, data->TimecodeSubframe, szTimecode, 128);
	printf("Timecode : %s\n", szTimecode);

	// Other Markers
	printf("Other Markers [Count=%d]\n", data->nOtherMarkers);
	for(i=0; i < data->nOtherMarkers; i++)
	{
		printf("Other Marker %d : %3.2f\t%3.2f\t%3.2f\n",
			i,
			data->OtherMarkers[i][0],
			data->OtherMarkers[i][1],
			data->OtherMarkers[i][2]);
	}

	// Rigid Bodies
	printf("Rigid Bodies [Count=%d]\n", data->nRigidBodies);
	double theta = calculate_angle(data->RigidBodies[0], data->RigidBodies[1], data->RigidBodies[2]);
	//if (178 < theta < 180) {
	//	send_cmd('0');
	//}
	double dist_calc = calculate_distance(data->RigidBodies[0], data->RigidBodies[3]);
	printf("\n\n\nAngle is %Lf and distance is %Lf\n\n\n", theta*180/3.14, dist_calc);

	for(i=0; i < data->nRigidBodies; i++)
	{
		printf("Rigid Body [ID=%d  Error=%3.2f]\n", data->RigidBodies[i].ID, data->RigidBodies[i].MeanError);
		printf("\tx\ty\tz\tqx\tqy\tqz\tqw\n");
		printf("\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
			data->RigidBodies[i].x,
			data->RigidBodies[i].y,
			data->RigidBodies[i].z,
			data->RigidBodies[i].qx,
			data->RigidBodies[i].qy,
			data->RigidBodies[i].qz,
			data->RigidBodies[i].qw);

		printf("\tRigid body markers [Count=%d]\n", data->RigidBodies[i].nMarkers);
		for(int iMarker=0; iMarker < data->RigidBodies[i].nMarkers; iMarker++)
		{
			location_data[i] = data->RigidBodies[i];
            printf("\t\t");
            if(data->RigidBodies[i].MarkerIDs)
                printf("MarkerID:%d", data->RigidBodies[i].MarkerIDs[iMarker]);
            if(data->RigidBodies[i].MarkerSizes)
                printf("\tMarkerSize:%3.2f", data->RigidBodies[i].MarkerSizes[iMarker]);
            if(data->RigidBodies[i].Markers)
                printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n" ,
                    data->RigidBodies[i].Markers[iMarker][0],
                    data->RigidBodies[i].Markers[iMarker][1],
                    data->RigidBodies[i].Markers[iMarker][2]);
        }
	}

	// skeletons
	printf("Skeletons [Count=%d]\n", data->nSkeletons);
	for(i=0; i < data->nSkeletons; i++)
	{
		sSkeletonData skData = data->Skeletons[i];
		printf("Skeleton [ID=%d  Bone count=%d]\n", skData.skeletonID, skData.nRigidBodies);
		for(int j=0; j< skData.nRigidBodies; j++)
		{
			sRigidBodyData rbData = skData.RigidBodyData[j];
			printf("Bone %d\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\t%3.2f\n",
				rbData.ID, rbData.x, rbData.y, rbData.z, rbData.qx, rbData.qy, rbData.qz, rbData.qw );

			printf("\tRigid body markers [Count=%d]\n", rbData.nMarkers);
			for(int iMarker=0; iMarker < data->RigidBodies[i].nMarkers; iMarker++)
			{
				printf("\t\t");
				if(rbData.MarkerIDs)
					printf("MarkerID:%d", rbData.MarkerIDs[iMarker]);
				if(rbData.MarkerSizes)
					printf("\tMarkerSize:%3.2f", rbData.MarkerSizes[iMarker]);
				if(rbData.Markers)
					printf("\tMarkerPos:%3.2f,%3.2f,%3.2f\n" ,
					data->RigidBodies[i].Markers[iMarker][0],
					data->RigidBodies[i].Markers[iMarker][1],
					data->RigidBodies[i].Markers[iMarker][2]);
			}
		}
	}

	// labeled markers
	printf("Labeled Markers [Count=%d]\n", data->nLabeledMarkers);
	for(i=0; i < data->nLabeledMarkers; i++)
	{
		sMarker marker = data->LabeledMarkers[i];
		printf("Labeled Marker [ID=%d] [size=%3.2f] [pos=%3.2f,%3.2f,%3.2f]\n", marker.ID, marker.size, marker.x, marker.y, marker.z);
	}
	if (!init_called) {
		//init();
		init_called = true;
	}
	int destination = dest_cont.dests[dest_cont.step_num];
	if (destination >0) {
		double angle_to_dest = calculate_angle(location_data[0], location_data[1], location_data[destination]);
		double dist_to_dest = calculate_distance(location_data[0], location_data[destination]);
		printf("==================================== %f", dist_to_dest);
		if (dist_to_dest < .6) {
			send_cmd('0');
			state = '0';
			dest_cont.step_num += 1;
			return;
		}
		if ( angle_to_dest < 175 && state != 'R') {
			send_cmd('R');
			state = 'R';
			return;
		}  else if (angle_to_dest > 175) {
			if (state == 'R' || state == 'L') {
				send_cmd('0');
				state = '0';
			} else {
				if (state != 'D') {
					send_cmd('D');
				}
				state = 'D';
			}
		}
	}

}

// MessageHandler receives NatNet error/debug messages
void __cdecl MessageHandler(int msgType, char* msg)
{
	printf("\n%s\n", msg);
}

/* File writing routines */
void _WriteHeader(FILE* fp, sDataDescriptions* pBodyDefs)
{
	int i=0;

    if(!pBodyDefs->arrDataDescriptions[0].type == Descriptor_MarkerSet)
        return;
        
	sMarkerSetDescription* pMS = pBodyDefs->arrDataDescriptions[0].Data.MarkerSetDescription;

	fprintf(fp, "<MarkerSet>\n\n");
	fprintf(fp, "<Name>\n%s\n</Name>\n\n", pMS->szName);

	fprintf(fp, "<Markers>\n");
	for(i=0; i < pMS->nMarkers; i++)
	{
		fprintf(fp, "%s\n", pMS->szMarkerNames[i]);
	}
	fprintf(fp, "</Markers>\n\n");

	fprintf(fp, "<Data>\n");
	fprintf(fp, "Frame#\t");
	for(i=0; i < pMS->nMarkers; i++)
	{
		fprintf(fp, "M%dX\tM%dY\tM%dZ\t", i, i, i);
	}
	fprintf(fp,"\n");

}

void _WriteFrame(FILE* fp, sFrameOfMocapData* data)
{
	fprintf(fp, "%d", data->iFrame);
	for(int i =0; i < data->MocapData->nMarkers; i++)
	{
		fprintf(fp, "\t%.5f\t%.5f\t%.5f", data->MocapData->Markers[i][0], data->MocapData->Markers[i][1], data->MocapData->Markers[i][2]);
	}
	fprintf(fp, "\n");
}

void _WriteFooter(FILE* fp)
{
	fprintf(fp, "</Data>\n\n");
	fprintf(fp, "</MarkerSet>\n");
}

void resetClient()
{
	int iSuccess;

	printf("\n\nre-setting Client\n\n.");

	iSuccess = theClient->Uninitialize();
	if(iSuccess != 0)
		printf("error un-initting Client\n");

	iSuccess = theClient->Initialize(szMyIPAddress, szServerIPAddress);
	if(iSuccess != 0)
		printf("error re-initting Client\n");


}

double calculate_angle(sRigidBodyData trackable1, sRigidBodyData trackable2, sRigidBodyData trackable3){
	//create vector between trackale1 and trackable2; another vector between trackable2 and trackable3
	double vector_t1_to_t2[2];
	double vector_t1_to_t3[2];
	vector_t1_to_t2[0] = trackable2.x - trackable1.x;
	vector_t1_to_t2[1] = trackable2.z - trackable1.z;
	vector_t1_to_t3[0] = trackable3.x - trackable1.x;
	vector_t1_to_t3[1] = trackable3.z - trackable1.z;
	
	//computing the dot product
	double dot_product = vector_t1_to_t2[0] * vector_t1_to_t3[0] + vector_t1_to_t2[1] * vector_t1_to_t3[1];
	
	//computing the magnitude of the 2 vectors
	double magn_1_to_2 = sqrt(vector_t1_to_t2[0] * vector_t1_to_t2[0] + vector_t1_to_t2[1] * vector_t1_to_t2[1]);
	double magn_1_to_3 = sqrt(vector_t1_to_t3[0] * vector_t1_to_t3[0] + vector_t1_to_t3[1] * vector_t1_to_t3[1]);
	
	//computing the angle using the arccosine
	double formula = dot_product/(magn_1_to_2 * magn_1_to_3); 
	double theta = acos(formula);

	printf("\n\ndot_product: %f, magnitude1: %f, magnitude2: %f, formula: %f, angle: %f\n\n\n", dot_product, magn_1_to_2, magn_1_to_3, formula, theta*180/PI);
	
	return theta*180/PI;
}

double calculate_distance(sRigidBodyData trackable1, sRigidBodyData trackable2){

	double distance = sqrt((trackable1.x - trackable2.x)*(trackable1.x - trackable2.x) + 
		(trackable1.z - trackable2.z)*(trackable1.z - trackable2.z));
	return distance;
}

//SampleClient 10.10.67.65 10.10.66.30