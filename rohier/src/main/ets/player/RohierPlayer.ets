import { RohierPlayerNapi } from '../napi/RohierPlayerNapi'
import { util } from '@kit.ArkTS'

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

    public prepare(dest: string, isUrl: boolean) {
        this.napi?.prepare(this.id, dest, isUrl)
    }

    public play() {
        this.napi?.play(this.id)
    }
}