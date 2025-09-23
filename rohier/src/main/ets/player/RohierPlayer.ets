import { RohierPlayerNapi } from '../napi/RohierPlayerNapi'
import { util } from '@kit.ArkTS'
import { VideoSource } from './VideoSource'

export class RohierPlayer {
    readonly id: string = util.generateRandomUUID()
    readonly type: XComponentType = XComponentType.SURFACE
    readonly libraryname: string = 'rohier'
    private napi?: RohierPlayerNapi

    constructor() {
    }

    public init(napi: object) {
        this.napi = napi as RohierPlayerNapi
        this.napi.init(this.id)
    }

    public prepare(source: VideoSource) {
        this.napi?.prepare(this.id, source.path, source.isUrl)
    }

    public play() {
        this.napi?.play(this.id)
    }

    public pause() {
        this.napi?.pause(this.id)
    }

    public seek(position: number) {
        this.napi?.seek(this.id, position)
    }

    public release() {
        this.napi?.release(this.id)
    }

}