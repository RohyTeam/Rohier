import { RohierPlayer } from '@rohy/rohier'
import { fileUri, picker } from '@kit.CoreFileKit'
import { UIContext } from '@kit.ArkUI'

@Entry
@ComponentV2
struct Index {
    @Local
    private player: RohierPlayer = new RohierPlayer()

    build() {
        Column() {
            XComponent(this.player)
                .onLoad((napi) => {
                    this.player.init(napi!)
                })
                .width('100%')
                .aspectRatio(4 / 3)
            this.button(
                "播放视频",
                () => {
                    this.playLocalVideo()
                }
            )
            Row() {
                Column() {
                    this.button(
                        "播放",
                        () => {
                            this.player.play()
                        }
                    )
                    this.button(
                        "暂停",
                        () => {
                            this.player.pause()
                        }
                    )
                    this.button(
                        "关闭",
                        () => {
                            this.player.release()
                        }
                    )
                }
                .layoutWeight(1)
                Column() {
                    this.button(
                        "跳转到第 15 秒",
                        () => {
                            this.player.seek(15 * 1000)
                        }
                    )
                    this.button(
                        "跳转到第 3 分",
                        () => {
                            this.player.seek(3 * 60 * 1000)
                        }
                    )
                }
                .layoutWeight(1)
            }
        }
        .height('100%')
        .width('100%')
    }

    @Builder
    private button(name: ResourceStr, action: () => void) {
        Button(name)
            .width("90%")
            .height(40)
            .margin({
                top: 16
            })
            .onClick(() => {
                action()
            })
    }

    private playLocalVideo() {
        pickSingleVideoFile(this.getUIContext())
            .then((url) => {
                if (url === undefined) {
                    return
                }
                this.player.prepare({
                    path: ensurePath(url),
                    isUrl: false
                })
                this.player.play()
            })
    }

}

export function pickSingleVideoFile(context: UIContext | Context): Promise<string | undefined> {
    return new Promise(async (resultReturner) => {
        let documentViewPicker = new picker.DocumentViewPicker(ensureContext(context));
        const documentSelectOptions = new picker.DocumentSelectOptions();
        documentSelectOptions.maxSelectNumber = 1;
        documentSelectOptions.fileSuffixFilters = [
            '全部视频文件|.mp4,.mkv,.mov,.avi,.flv',
            'mp4 视频文件|.mp4',
            'mkv 视频文件|.mkv',
            'mov 视频文件|.mov',
            'avi 视频文件|.avi',
            'flv 视频文件|.flv'
        ];

        let documentSelectResult = await documentViewPicker.select(documentSelectOptions)

        if (documentSelectResult.length > 0) {
            resultReturner(documentSelectResult[0])
        } else {
            resultReturner(undefined)
        }
    })
}

export function ensureContext(context: UIContext | Context): Context {
    if (context instanceof UIContext) {
        return context.getHostContext()!
    } else {
        return context
    }
}

export function ensurePath(uriOrPath: string): string {
    return new fileUri.FileUri(uriOrPath).path
}