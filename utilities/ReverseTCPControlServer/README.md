# Reverse TCP Control Server

This project sets up a proxy server that allows an **ESP32** device to communicate with a **client** (such as a web browser or other HTTP client) via a **Python asyncio server**. 

The server accepts connections from both the ESP32 and client, forwarding requests and responses between them.

## Features

- Manages connections between an ESP32 and client.
- Forwards client requests to the ESP32 and streams ESP32 responses back to the client.
- Utilizes asyncio for asynchronous handling of connections.
- Allows multiple clients to connect to the ESP32 through a single proxy server.

## Prerequisites

- Python 3.7 or higher
- ESP32 device (configured to connect to this server)
- Basic knowledge of networking, as NAT configuration is required for external access.

## Setup

### 1. Install Python Dependencies

Make sure you have Python 3.7+ installed. Then, install any additional dependencies (if any are needed for future expansion):

```bash
pip install asyncio
