export declare class RohierPlayerNapi {

    init(id: string): void
    prepare(id: string, dest: string, isUrl: boolean): void
    play(id: string): void
    pause(id: string): void
    seek(id: string, position: number): void
    release(id: string): void

}