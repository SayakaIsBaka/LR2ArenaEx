#include <iostream>
#include <sstream>
#include <network/enums.h>
#include <utils/msgpack_utils.h>

#include "server.h"

void ResetState(network::Address player) {
    server::state.peers[player].ready = false;
    server::state.peers[player].selectedHash = "";
    server::state.peers[player].score = network::Score();
}

void ResetStateEveryone() {
    for (const auto& [key, value] : server::state.peers)
    {
        // Reset state as new chart has been selected
        ResetState(key);
    }
}

void ParseSelectedBms(std::vector<char> data, network::Address clientAddr) {
    auto selectedBms = msgpack_utils::unpack<network::SelectedBmsMessage>(data);

    if (clientAddr == server::state.host) {
        server::state.currentRandomSeed = selectedBms.randomSeed;
        server::state.itemModeEnabled = selectedBms.itemModeEnabled;
        ResetStateEveryone();
    }

    std::cout << "[server] Received selected bms: " << selectedBms.title << " / " << selectedBms.artist << std::endl;
    std::cout << "[server] Hash: " << selectedBms.hash << std::endl;
    std::cout << "[server] Random seed: " << server::state.currentRandomSeed << std::endl;

    server::state.peers[clientAddr].selectedHash = selectedBms.hash;
    server::state.peers[clientAddr].option = selectedBms.option;
    server::state.peers[clientAddr].gauge = selectedBms.gauge;
}

void ParseScore(std::vector<char> data, network::Address clientAddr) {
    auto score = msgpack_utils::unpack<network::Score>(data);
    server::state.peers[clientAddr].score = score;
}

void SetUsername(std::vector<char> data, network::Address clientAddr) { // Performing state update here to avoid race condition
    std::string username(data.begin(), data.end());
    std::cout << "[server] Username: " << username << std::endl;
    if (server::state.peers.size() == 0) { // If first user to connect, set as host
        server::state.host = clientAddr;
    }
    server::state.peers[clientAddr] = network::Peer();
    server::state.peers[clientAddr].username = username;
}

void SetHost(std::vector<char> data) {
    auto newHost = msgpack_utils::unpack<network::Address>(data);
    if (server::state.peers.find(newHost) == server::state.peers.end()) {
        std::cout << "[!][server] Player not found for new host" << std::endl;
        return;
    }
    server::state.host = newHost;
}

bool IsEveryoneReady() {
    bool allReady = true;
    std::string hash = server::state.peers.begin()->second.selectedHash; // Take first hash as reference, all players must have the same chart selected anyways so no difference
    for (const auto& [key, value] : server::state.peers) {
        allReady = server::state.peers[key].ready && server::state.peers[key].selectedHash == hash;
        if (!allReady) break;
    }
    return allReady;
}

void AutoRotateHost() {
    auto currentHost = server::state.host;
    bool isNext = false;
    network::Address first;
    for (const auto [key, value] : server::state.peers) {
        if (first.host.empty())
            first = key;
        if (isNext) {
            isNext = false;
            server::state.host = key;
            break;
        }
        if (key == currentHost)
            isNext = true;
    }
    if (isNext)
        server::state.host = first;
}

void SetItemSettings(std::vector<char> data) {
    auto itemSettings = msgpack_utils::unpack<network::ItemSettings>(data);
    server::state.itemSettings = itemSettings;
}

void KickUser(network::Address userToKick) {
    for (const auto& c : server::server->getClients())
    {
        std::string userUrl = userToKick.host + ":" + std::to_string(userToKick.port);
        if (userUrl == c->getUrl())
            c->close();
    }
}

void server::SendToEveryone(network::ServerToClient id, std::vector<char> data, network::Address origSenderAddr, bool includeOrigSender) {
    data.insert(data.begin(), static_cast<char>(id));
    for (const auto& addr : server->getClients())
    {
        std::string origSenderUrl = origSenderAddr.host + ":" + std::to_string(origSenderAddr.port);
        if (origSenderUrl == addr->getUrl() && !includeOrigSender)
           continue;
        addr->sendBinary(data);
    }
}

void server::SendTo(network::ServerToClient id, std::vector<char> data, network::Address addr) {
    data.insert(data.begin(), static_cast<char>(id));
    for (const auto& c : server->getClients())
    {
        std::string addrUrl = addr.host + ":" + std::to_string(addr.port);
        if (addrUrl == c->getUrl())
            c->sendBinary(data);
    }
}

void server::ParsePacket(std::vector<char> data, network::Address clientAddr) {
	char id = data.front();
	data.erase(data.begin());

	switch ((network::ClientToServer)id)
	{
	case network::ClientToServer::CTS_SELECTED_BMS:
        ParseSelectedBms(data, clientAddr);
        if (clientAddr == state.host)
            SendToEveryone(network::ServerToClient::STC_SELECTED_CHART_RANDOM, data, clientAddr, true);
        // If not host, send random + hash + received BMS to other clients; otherwise do nothing?
		break;
	case network::ClientToServer::CTS_PLAYER_SCORE:
        std::cout << "[server] Received player score" << std::endl;
        ParseScore(data, clientAddr);
        SendToEveryone(network::ServerToClient::STC_PLAYERS_SCORE, msgpack_utils::pack(network::ScoreMessage(state.peers[clientAddr].score, clientAddr)), clientAddr, true);
		break;
	case network::ClientToServer::CTS_CHART_CANCELLED:
        std::cout << "[server] Received chart cancelled" << std::endl;
        server::state.peers[clientAddr].ready = false;
        server::state.peers[clientAddr].selectedHash = "";
        SendToEveryone(network::ServerToClient::STC_PLAYERS_READY_UPDATE, msgpack_utils::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
		break;
	case network::ClientToServer::CTS_LOADING_COMPLETE:
        std::cout << "[server] Received loading complete from " << clientAddr.host << std::endl;
        server::state.peers[clientAddr].ready = true;
        if (autoRotateHost && IsEveryoneReady())
            AutoRotateHost();
        SendToEveryone(network::ServerToClient::STC_PLAYERS_READY_UPDATE, msgpack_utils::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
		break;
    case network::ClientToServer::CTS_USERNAME:
        SetUsername(data, clientAddr);
        SendTo(network::ServerToClient::STC_CLIENT_REMOTE_ID, msgpack_utils::pack(clientAddr), clientAddr); // Send remote address to sender (use as ID)
        if (!server::state.itemSettings.settings.empty())
            SendTo(network::ServerToClient::STC_ITEM_SETTINGS, msgpack_utils::pack(server::state.itemSettings), clientAddr); // Send custom item settings if defined
        SendToEveryone(network::ServerToClient::STC_USERLIST, msgpack_utils::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
        break;
    case network::ClientToServer::CTS_MESSAGE:
        std::cout << "[server] Received message" << std::endl;
        SendToEveryone(network::ServerToClient::STC_MESSAGE, msgpack_utils::pack(network::Message(std::string(data.begin(), data.end()), clientAddr, false)), clientAddr, false);
        break;
    case network::ClientToServer::CTS_MISSING_CHART:
        std::cout << "[server] Received missing chart" << std::endl;
        SendToEveryone(network::ServerToClient::STC_MESSAGE, msgpack_utils::pack(network::Message("[!] " + state.peers[clientAddr].username + " is missing the selected chart!", clientAddr, true)), clientAddr, false);
        break;
    case network::ClientToServer::CTS_SET_HOST:
        std::cout << "[server] Received set host" << std::endl;
        if (state.host == clientAddr) {
            SetHost(data);
            SendToEveryone(network::ServerToClient::STC_USERLIST, msgpack_utils::pack(network::PeerList(state.peers, state.host)), clientAddr, true);
        }
        else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
    case network::ClientToServer::CTS_KICK_USER:
        std::cout << "[server] Received kick user" << std::endl;
        if (state.host == clientAddr) {
            KickUser(msgpack_utils::unpack<network::Address>(data));
        }
        else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
    case network::ClientToServer::CTS_ITEM:
        std::cout << "[server] Received item use" << std::endl;
        SendToEveryone(network::ServerToClient::STC_ITEM, data, clientAddr, false);
        break;
    case network::ClientToServer::CTS_ITEM_SETTINGS:
        std::cout << "[server] Received item settings" << std::endl;
        if (state.host == clientAddr) {
            SetItemSettings(data);
            SendToEveryone(network::ServerToClient::STC_ITEM_SETTINGS, data, clientAddr, false);
        } else {
            std::cout << "[!][server] Sender is not the host!" << std::endl;
        }
        break;
	default:
        std::cout << "[server] Unknown message received" << std::endl;
		break;
	}
}

void server::ClientConnected(network::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") connected." << std::endl;
}

void server::ClientDisconnected(network::Address clientAddr)
{
    std::cout << "[server] Client (" + clientAddr.host + ":" + std::to_string(clientAddr.port) + ") disconnected." << std::endl;
    if (started) {
        state.peers.erase(clientAddr);
        if (clientAddr == state.host && state.peers.size() > 0)
            state.host = state.peers.begin()->first; // Change host to first peer in the list

        if (state.peers.size() > 0) {
            auto res = msgpack_utils::pack(network::PeerList(state.peers, state.host));
            res.insert(res.begin(), (char)network::ServerToClient::STC_USERLIST);
        
            for (const auto& addr : server->getClients())
            {
                std::string clientUrl = clientAddr.host + ":" + std::to_string(clientAddr.port);
                if (clientUrl == addr->getUrl()) continue;
                addr->sendBinary(res);
            }
        }
    }
}

void server::OnClientMessageReceived(std::shared_ptr<ix::ConnectionState> connectionState, ix::WebSocket& webSocket, const ix::WebSocketMessagePtr& msg) {
    network::Address clientAddr;
    clientAddr.host = connectionState->getRemoteIp();
    clientAddr.port = connectionState->getRemotePort();

    if (msg->type == ix::WebSocketMessageType::Open) {
        webSocket.setUrl(clientAddr.host + ":" + std::to_string(clientAddr.port)); // URL is unused on client sockets, use it to store host + port
        ClientConnected(clientAddr);
    }
    else if (msg->type == ix::WebSocketMessageType::Message) {
        std::vector<char> data(msg->str.begin(), msg->str.end());
        ParsePacket(data, clientAddr);
    }
    else if (msg->type == ix::WebSocketMessageType::Close || msg->type == ix::WebSocketMessageType::Error) {
        ClientDisconnected(clientAddr);
    }
    else if (msg->type == ix::WebSocketMessageType::Fragment) {
        std::cout << "[!] Received websocket fragment, unsupported" << std::endl;
    }
}

bool server::Start(const char* host, unsigned short port) {
    // Hopefully this is enough to avoid a memleak because delete on a regular pointer crashes the thing
    server = std::make_shared<ix::WebSocketServer>(port, host, ix::SocketServer::kDefaultTcpBacklog, ix::SocketServer::kDefaultMaxConnections, ix::WebSocketServer::kDefaultHandShakeTimeoutSecs, ix::SocketServer::kDefaultAddressFamily, 15);
    server->setOnClientMessageCallback(OnClientMessageReceived);
    auto success = server->listen();

    if (!success.first)
    {
        std::cout << "[!] Error creating server" << std::endl;
        return false;
    }

    started = true;
    server->disablePerMessageDeflate();
    server->start();

    std::cout << "[+] Started server on port " << std::dec << port << std::endl;

    return true;
}

bool server::Stop() {
    server->stop();
    started = false;
    state = State();
    std::cout << "[+] Stopped server" << std::endl;
    return true;
}