import { socket } from "@kit.NetworkKit";
import { BusinessError } from "@kit.BasicServicesKit";
import { hilog } from "@kit.PerformanceAnalysisKit";

export class RohierProxyServer {

    private server: socket.TCPSocketServer
    private port: number = 52940

    constructor() {
        this.server = socket.constructTCPSocketServerInstance()
        this.server.listen({
            address: '127.0.0.1',
            port: 52940
        })
        this.server.getLocalAddress()
            .then((address) => {
                this.port = address.port!
            })
            .catch((err: BusinessError) => {
                hilog.error(0, 'RohierProxyServer', `Failed to create Rohier proxy server: ${err.message} (${err.code})`)
            })
        this.server.on('connect', async (client: socket.TCPSocketConnection) => {

        })
    }

    public mirrorUrl(rawUrl: string, headers?: Record<string, string>) {
        
    }

}