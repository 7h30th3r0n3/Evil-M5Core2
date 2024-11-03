import asyncio

ESP32_HOST = '0.0.0.0'
ESP32_PORT = 4444

CLIENT_HOST = '0.0.0.0'
CLIENT_PORT = 80

class ESP32Proxy:
    def __init__(self):
        self.esp32_reader = None
        self.esp32_writer = None
        self.esp32_connected = asyncio.Event()

    async def handle_esp32(self, reader, writer):
        print('ESP32 connected')
        self.esp32_reader = reader
        self.esp32_writer = writer
        self.esp32_connected.set()

        try:
            while not reader.at_eof():
                await asyncio.sleep(1)
        except Exception as e:
            print(f"Exception in handle_esp32: {e}")
        finally:
            print('ESP32 disconnected')
            self.esp32_reader = None
            self.esp32_writer = None
            self.esp32_connected.clear()

    async def handle_client(self, reader, writer):
        await self.esp32_connected.wait()
        print("_______________________________________________________________________")
        print("Client connected, waiting for request...")

        try:
            # Read the client's request
            request = await reader.read(8192)
            print(f"Request received from client: {request.decode('utf-8', errors='replace')}")

            # Send the request to the ESP32
            if self.esp32_writer is not None:
                self.esp32_writer.write(request)
                await self.esp32_writer.drain()
                print("Request sent to ESP32, waiting for response...")

            # Transfer the response from ESP32 to the client in real time
            while True:
                chunk = await asyncio.wait_for(self.esp32_reader.read(8192), timeout=1)
                if not chunk:
                    break
                writer.write(chunk)
                await writer.drain()
        except asyncio.TimeoutError:
            pass
        except BrokenPipeError:
            print("Connection closed by client (BrokenPipeError).")
        except Exception as e:
            print(f"Exception while transferring the response: {e}")
        finally:
            print("Response sent to client.")
            print("_______________________________________________________________________")
            try:
                writer.close()
                await writer.wait_closed()
            except Exception as e:
                print(f"Exception while closing the writer: {e}")

    async def main(self):
        esp32_server = await asyncio.start_server(
            self.handle_esp32, ESP32_HOST, ESP32_PORT)
        client_server = await asyncio.start_server(
            self.handle_client, CLIENT_HOST, CLIENT_PORT)

        print(f"Server listening for ESP32 on port {ESP32_PORT}")
        print(f"Server listening for clients on port {CLIENT_PORT}")

        async with esp32_server, client_server:
            await asyncio.gather(esp32_server.serve_forever(), client_server.serve_forever())

if __name__ == '__main__':
    proxy = ESP32Proxy()
    asyncio.run(proxy.main())
