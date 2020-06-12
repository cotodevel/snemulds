using System;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Text;
using System.Collections.Generic;
using System.Net.Sockets;

namespace ConsoleApplication1
{

    //UDP Defs

    public struct Received
    {
        public IPEndPoint Sender;
        public string Message;
    }

    abstract class UdpBase
    {
        protected UdpClient Client;

        protected UdpBase()
        {
            Client = new UdpClient();
        }

        public async Task<Received> Receive()
        {
            var result = await Client.ReceiveAsync();
            return new Received()
            {
                Message = Encoding.ASCII.GetString(result.Buffer, 0, result.Buffer.Length),
                Sender = result.RemoteEndPoint
            };
        }
    }

    //Server
    class UdpListener : UdpBase
    {
        private IPEndPoint _listenOn;

        public UdpListener()
            : this(new IPEndPoint(IPAddress.Any, 8888))
        {
        }

        public UdpListener(IPEndPoint endpoint)
        {
            _listenOn = endpoint;
            Client = new UdpClient(_listenOn);
        }

        public void Reply(string message, IPEndPoint endpoint)
        {
            var datagram = Encoding.ASCII.GetBytes(message);
            Client.Send(datagram, datagram.Length, endpoint);
        }

    }

    //Client
    class UdpUser : UdpBase
    {
        private UdpUser() { }

        public static UdpUser ConnectTo(string hostname, int port)
        {
            var connection = new UdpUser();
            connection.Client.Connect(hostname, port);
            return connection;
        }

        public void Send(string message)
        {
            var datagram = Encoding.ASCII.GetBytes(message);
            Client.Send(datagram, datagram.Length);
        }

    }

    class DS_STAT_MODES
    {
        public static string idle = "idle-lookup-server";
        public static string handshake = "ds-server-handshake";
        public static string connected = "connected-dswnifi";
    }

    //each DS connected here
    class NDS_MULTI_SESSION
    {
        public string ip { get; set; }
        public string port_sender { get; set; }     //sender: sends commands to server here
        public string port_listener { get; set; }   //listener: recvs commands from server here
        public string server_port_cmd { get; set; }
        public bool status_multiplay { get; set; }  //false == idle / true == multiplay
        public string mode;    //host / guest

        public string ds_status;    //idle-lookup-server, ds-server-handshake - connected-dswnifi
        public string connection_method;    //TCP / UDP
    }

    class Program
    {
        //UDP Sender
        public static void SendUDPMsg(string msg, string ip_target, string udp_port)
        {
            Socket s = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            IPAddress broadcast = IPAddress.Parse(ip_target);
            string udpmsg = msg;
            byte[] sendbuf = Encoding.ASCII.GetBytes(udpmsg);
            IPEndPoint ep = new IPEndPoint(broadcast, Convert.ToInt32(udp_port));
            s.SendTo(sendbuf, ep);
        }

        //TCP Sender
        public static void SendTCPMsg(string msg, string ip_target, string tcp_port)
        {    
            TcpClient ourMagicClient = new TcpClient();
            ourMagicClient.Connect(ip_target, Convert.ToInt32(tcp_port));
            NetworkStream ourStream = ourMagicClient.GetStream();
            byte[] data = Encoding.ASCII.GetBytes(msg);
            ourStream.Write(data, 0, data.Length); //Start at the 0'th position in our string and send until the end of the string.
        }

        static private string LocalIPAddress()
        {
            string localIP = String.Empty;
            try
            {
                IPHostEntry host;
                host = Dns.GetHostEntry(Dns.GetHostName());
                foreach (IPAddress ip in host.AddressList)
                {
                    if (ip.AddressFamily.ToString() == "InterNetwork")
                    {
                        localIP = ip.ToString();
                    }
                }
                return localIP;
            }
            catch (Exception ex)
            {
                //LogWriter.WriteExceptionLog(ex.ToString());
            }

            return localIP;
        }

        static void Main(string[] args)
        {
            //create multi session
            List<NDS_MULTI_SESSION> nds_session_list = new List<NDS_MULTI_SESSION>();
            int server_port = 0;
            //create a new server
            var server = new UdpListener();

            Console.ForegroundColor = ConsoleColor.Yellow;
            Console.WriteLine(" UDP TGDS Server Companion Program. \n Usage: Connect this computer to the same network a DS is setup. \n Any connected DS through the DSWNIFI library should connect automatically.\n Developed by coto. Press ESC to quit");

            //test UDP RECV on DS: SendUDPMsg("hi", "192.168.43.128", "8890");

            //start listening for messages and copy the messages back to the client
            Task.Factory.StartNew(async () =>
            {
                while (true)
                {
                    var received = await server.Receive();
                    server.Reply(received.Message, received.Sender);
                    string[] str_list = received.Message.Split('-');

                    //for assigning ids
                    Random random_inst = new Random();

                    //Console.WriteLine("UDPMSG:" + received.Message + " \n");
                    //Console.WriteLine(str_list[0]+" \n");             //cmd
                    //Console.WriteLine(str_list[1] + " \n");           //header
                    //Console.WriteLine(str_list[2] + " \n");           //IP
                    //Console.WriteLine(str_list[3] + " \n ");   //NDS MULTI PORT

                    string ConnMethod = String.Empty;

                    if (str_list[3].ToString().Equals("TCP"))
                    {
                        ConnMethod = "TCP";
                        server_port = 7777;
                    }
                    else {
                        ConnMethod = "UDP";
                        server_port = 8888;
                    }

                    string incomingnds_cmd = str_list[0].Trim();
                    string incomingnds_ip = str_list[2].Trim();
                    string incomingnds_multi_port;

                    if (str_list[0] == "dsnotaware")
                    {
                        bool rec_already_exists = false;

                        int index_found = 0;
                        foreach (NDS_MULTI_SESSION entry in nds_session_list)
                        {
                            if (entry.ip.Equals(incomingnds_ip))
                            {
                                rec_already_exists = true;
                                break;
                            }
                            index_found++;
                        }

                        if (rec_already_exists == false)
                        {
                            nds_session_list.Add(new NDS_MULTI_SESSION { ip = incomingnds_ip, status_multiplay = false, server_port_cmd = server_port.ToString().Trim(), ds_status = DS_STAT_MODES.idle, connection_method = ConnMethod });
                            Console.WriteLine("DS's detected so far:" + nds_session_list.Count() + "\n");
                        }
                        else
                        {
                            //Console.Write("DS [" + nds_session_list[index_found].ip + ":" + nds_session_list[index_found].port + "] already registered");
                        }

                        //2 ds are connected and waiting
                        if (nds_session_list.Count() > 1)
                        {
                            int nds1_index = 0;
                            int nds2_index = 0;

                            foreach (NDS_MULTI_SESSION entry in nds_session_list)
                            {
                                if (entry.status_multiplay == false)
                                {
                                    break;
                                }
                                nds1_index++;
                            }

                            foreach (NDS_MULTI_SESSION entry in nds_session_list)
                            {
                                if ((entry.status_multiplay == false) && !nds_session_list[nds1_index].Equals(entry))
                                {
                                    break;
                                }
                                nds2_index++;
                            }

                            //found them
                            if ((nds1_index != nds2_index) && (nds_session_list[nds1_index].status_multiplay == false) && (nds_session_list[nds2_index].status_multiplay == false))
                            {
                                string ds1_mode = "notassigned";
                                string ds2_mode = "notassigned";

                                // ind_token : 0 == guest  | 1 == host
                                //id 0 is null / id 1 == host / id 2 == guest
                                int ind_token = random_inst.Next(1);
                                if (ind_token == 0)
                                {
                                    //nds1_index[console1] is guest
                                    nds_session_list[nds1_index].mode = ds1_mode = "guest";
                                    //nds2_index[console2] is host
                                    nds_session_list[nds2_index].mode = ds2_mode = "host";
                                }
                                else
                                {
                                    //nds1_index[console1] is host
                                    nds_session_list[nds1_index].mode = ds1_mode = "host";
                                    //nds2_index[console2] is guest
                                    nds_session_list[nds2_index].mode = ds2_mode = "guest";
                                }

                                //assign ports
                                int LISTENER_PORT = 0;
                                int SENDER_PORT = 0;
                                if (ds1_mode == "host")
                                {
                                    //TCP
                                    if (nds_session_list[nds1_index].connection_method.Equals("TCP"))
                                    {
                                        LISTENER_PORT = 7778;       //NDSMULTI_TCP_PORT_HOST 7778
                                        SENDER_PORT = 7779;         //NDSMULTI_TCP_PORT_GUEST 7779
                                    }
                                    //UDP
                                    else
                                    {
                                        LISTENER_PORT = 8889;       //NDSMULTI_UDP_PORT_HOST 8889
                                        SENDER_PORT = 8890;         //NDSMULTI_UDP_PORT_GUEST 8890
                                    }
                                    nds_session_list[nds1_index].port_listener = LISTENER_PORT.ToString();
                                    nds_session_list[nds1_index].port_sender = SENDER_PORT.ToString();

                                    nds_session_list[nds2_index].port_listener = SENDER_PORT.ToString();
                                    nds_session_list[nds2_index].port_sender = LISTENER_PORT.ToString();
                                }
                                else if (ds2_mode == "host")
                                {
                                    //TCP
                                    if (nds_session_list[nds2_index].connection_method.Equals("TCP"))
                                    {
                                        LISTENER_PORT = 7778;       //NDSMULTI_TCP_PORT_HOST 7778
                                        SENDER_PORT = 7779;         //NDSMULTI_TCP_PORT_GUEST 7779
                                    }
                                    //UDP
                                    else {
                                        LISTENER_PORT = 8889;    //NDSMULTI_UDP_PORT_HOST 8889
                                        SENDER_PORT = 8890;     //NDSMULTI_UDP_PORT_GUEST 8890
                                    }
                                    nds_session_list[nds2_index].port_listener = LISTENER_PORT.ToString();
                                    nds_session_list[nds2_index].port_sender = SENDER_PORT.ToString();

                                    nds_session_list[nds1_index].port_listener = SENDER_PORT.ToString();
                                    nds_session_list[nds1_index].port_sender = LISTENER_PORT.ToString();
                                }

                                //Console.Clear();

                                //send to DS's: MULTI IP of each counterpart DS, assign the new host-guest mode for each
                                string ip_dest_ds1 = String.Empty;
                                ip_dest_ds1 = nds_session_list[nds2_index].ip;  //guest <- ip host

                                string ip_dest_ds2 = String.Empty;
                                ip_dest_ds2 = nds_session_list[nds1_index].ip;  //guest <- ip host

                                nds_session_list[nds1_index].status_multiplay = true;
                                nds_session_list[nds2_index].status_multiplay = true;

                                //UDP Method: Connect
                                if ((nds_session_list[nds1_index].connection_method == "UDP") && (nds_session_list[nds2_index].connection_method == "UDP"))
                                {
                                    int udp_port = 8888;    //same as #define UDP_PORT 8888
                                    //DS1
                                    string udpmsgds1 = "srvaware" + "-" + ip_dest_ds1 + "-" + ds1_mode + "-";
                                    //DS2
                                    string udpmsgds2 = "srvaware" + "-" + ip_dest_ds2 + "-" + ds2_mode + "-";

                                    SendUDPMsg(udpmsgds1, nds_session_list[nds1_index].ip, udp_port.ToString());
                                    Thread.Sleep(4000);
                                    SendUDPMsg(udpmsgds2, nds_session_list[nds2_index].ip, udp_port.ToString());
                                    Console.WriteLine("UDP: DS's are binded correctly :)");
                                }
                                else if ((nds_session_list[nds1_index].connection_method == "TCP") && (nds_session_list[nds2_index].connection_method == "TCP")) {
                                    int tcp_port = 7777;    //same as #define TCP_PORT 7777
                                    string tcpmsgds1 = "srvaware" + "-" + ip_dest_ds1 + "-" + ds1_mode + "-";
                                    SendTCPMsg(tcpmsgds1, nds_session_list[nds1_index].ip, tcp_port.ToString());
                                    Thread.Sleep(488);  //4s for TCP
                                    //DS2
                                    string tcpmsgds2 = "srvaware" + "-" + ip_dest_ds2 + "-" + ds2_mode + "-";
                                    SendTCPMsg(tcpmsgds2, nds_session_list[nds2_index].ip, tcp_port.ToString());
                                    Console.WriteLine("TCP: DS's are binded correctly :)");
                                }
                                
                            }
                            else
                            {
                                //Console.WriteLine("not enough NDSs available for multi IDLE'd");
                            }

                        }

                        //Console.WriteLine("DSCONSOLEWANTSTOSIGN-IN. DS's so far:"+nds_session_list.Count()+"\n");
                    }



                    if (str_list[0] == "dsaware")
                    {
                        //ds cmd: dsaware-%s-bindOK-%d-%s- (IP of NDS that sent this request)   
                        //str_list[0] //cmd
                        //str_list[1] //host-guest assigned
                        //str_list[2] //binding status
                        //str_list[3] //DS listener port for recv cmds
                        //str_list[4] //IP
                        //Console.WriteLine("DSOK:[" + str_list[1] + "]-[" + str_list[2] + "]-[" + str_list[4] + "]:[" + str_list[3] + "] \n");

                        int nds_index = 0;
                        string found_ip = String.Empty;
                        //get index(DS Console that sent this msg) from IP
                        foreach (NDS_MULTI_SESSION reg in nds_session_list)
                        {
                            if (((reg.ip + "\n") == str_list[4]) && (nds_session_list[nds_index].ds_status != DS_STAT_MODES.handshake))  //lazy I know but works anyway
                            {
                                //nds_session_list[nds_index].ds_status = DS_STAT_MODES.handshake;
                                found_ip = reg.ip;
                                break;
                            }
                            nds_index++;
                        }

                        //is this DS actually registered earlier? If so, proceed to connect
                        if (found_ip != String.Empty)
                        {
                            //UDP Method: Connect
                            if(nds_session_list[nds_index].connection_method == "UDP")
                            {
                                //send "dsconnect-" //where IP sent is the one this DS must connect to.
                                SendUDPMsg("dsconnect-", nds_session_list[nds_index].ip.Trim(), nds_session_list[nds_index].server_port_cmd.Trim());
                            }
                            //TCP Method: Connect
                            else if (nds_session_list[nds_index].connection_method == "TCP")
                            {
                                SendTCPMsg("dsconnect-", nds_session_list[nds_index].ip.Trim(), nds_session_list[nds_index].server_port_cmd.Trim());
                            }
                        }
                    }

                    if (str_list[0] == "dsconnected")
                    {
                        //update lib status here

                    }

                    if (str_list[0] == "debug")
                    {
                        Console.WriteLine("DEBUG:" + received.Message + " \n");
                    }

                    if (received.Message == "quit")
                        break;
                }
            });


            /* TCP Client, connect to example 
            const int ourPort = 55555; // This needs to match the server's port

            TcpClient ourMagicClient = new TcpClient();

            //Connect to the server - change this IP address to match your server's IP!
            ourMagicClient.Connect("192.168.43.128", ourPort);

            //Use a NetworkStream object to send and/or receive some data
            NetworkStream ourStream = ourMagicClient.GetStream();

            //Let's set up some data!
            byte[] data = Encoding.ASCII.GetBytes("What a cute widdle robot you have there!\n");
            //Everyone ready? Send that bad-boy! (sender part)
            ourStream.Write(data, 0, data.Length); //Start at the 0'th position in our string and send until the end of the string, but we can stop there...

            Console.WriteLine("trying TCP send");
            ourStream.Close();

            while (1==1) ;
            */
            
            /* TCP Server/Listener example
            IPAddress ipAddress = IPAddress.Parse(LocalIPAddress());

            Console.WriteLine("Starting TCP listener...");

            TcpListener listener = new TcpListener(ipAddress, 7777);    //TCP_PORT 7777

            listener.Start();

            while (true)
            {
                Console.WriteLine("Server is listening on " + listener.LocalEndpoint);

                Console.WriteLine("Waiting for a connection...");

                
                Socket client = listener.AcceptSocket();

                Console.WriteLine("Connection accepted.");

                Console.WriteLine("Reading data...");

                byte[] datarecv = new byte[100];
                int size = client.Receive(datarecv);
                Console.WriteLine("Recieved data: ");
                for (int i = 0; i < size; i++)
                    Console.Write(Convert.ToChar(datarecv[i]));

                Console.WriteLine();

                client.Close();
            }

            listener.Stop();
            */

            while (ConsoleKey.Escape != Console.ReadKey().Key)
            {
                Thread.Sleep(100);  //100ms
            }
        }
             
    }

}