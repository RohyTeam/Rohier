import { RohierPlayerNapi } from "@rohy/rohier/src/main/cpp/types/librohier"
import { fileUri, picker } from "@kit.CoreFileKit"
import { UIContext } from "@kit.ArkUI"

@Entry
@ComponentV2
struct Index {

  @Local
  private player: RohierPlayerNapi = new RohierPlayerNapi()

  build() {
    Column() {
      XComponent({
        id: 'rohier_test',
        type: XComponentType.SURFACE,
        libraryname: 'rohier'
      })
        .onLoad(() => {
          this.player.init('rohier_test')
        })
        .width('100%')
        .aspectRatio(4 / 3)
      Button("播放视频")
        .width("90%")
        .height(40)
        .onClick(() => {
          pickSingleVideoFile(this.getUIContext())
            .then((url) => {
              if (url === undefined)
                return
              this.player.prepare(ensurePath(url), false)
              this.player.play()
            })
        })
    }
    .height('100%')
    .width('100%')
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