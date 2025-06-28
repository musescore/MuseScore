// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only

/**
 * Loads the instance of a WASM module.
 *
 * @param config May contain any key normally accepted by emscripten and the 'qt' extra key, with
 *               the following sub-keys:
 * - environment: { [name:string] : string }
 *      environment variables set on the instance
 * - onExit: (exitStatus: { text: string, code?: number, crashed: bool }) => void
 *      called when the application has exited for any reason. There are two cases:
 *      aborted: crashed is true, text contains an error message.
 *      exited: crashed is false, code contians the exit code.
 *
 *      Note that by default Emscripten does not exit when main() returns. This behavior
 *      is controlled by the EXIT_RUNTIME linker flag; set "-s EXIT_RUNTIME=1" to make
 *      Emscripten tear down the runtime and exit when main() returns.
 *
 * - containerElements: HTMLDivElement[]
 *      Array of host elements for Qt screens. Each of these elements is mapped to a QScreen on
 *      launch.
 * - fontDpi: number
 *      Specifies font DPI for the instance
 * - onLoaded: () => void
 *      Called when the module has loaded, at the point in time where any loading placeholder
 *      should be hidden and the application window should be shown.
 * - entryFunction: (emscriptenConfig: object) => Promise<EmscriptenModule>
 *      Qt always uses emscripten's MODULARIZE option. This is the MODULARIZE entry function.
 * - module: Promise<WebAssembly.Module>
 *      The module to create the instance from (optional). Specifying the module allows optimizing
 *      use cases where several instances are created from a single WebAssembly source.
 * - qtdir: string
 *      Path to Qt installation. This path will be used for loading Qt shared libraries and plugins.
 *      The path is set to 'qt' by default, and is relative to the path of the web page's html file.
 *      This property is not in use when static linking is used, since this build mode includes all
 *      libraries and plugins in the wasm file.
 * - preload: [string]: Array of file paths to json-encoded files which specifying which files to preload.
 *      The preloaded files will be downloaded at application startup and copied to the in-memory file
 *      system provided by Emscripten.
 *
 *      Each json file must contain an array of source, destination objects:
 *      [
 *           {
 *               "source": "path/to/source",
 *               "destination": "/path/to/destination"
 *           },
 *           ...
 *      ]
 *      The source path is relative to the html file path. The destination path must be
 *      an absolute path.
 *
 *      $QTDIR may be used as a placeholder for the "qtdir" configuration property (see @qtdir), for instance:
 *          "source": "$QTDIR/plugins/imageformats/libqjpeg.so"
 *  - localFonts.requestPermission: bool
 *       Whether Qt should request for local fonts access permission on startup (default false).
 *  - localFonts.familiesCollection string
 *       Specifies a collection of local fonts to load. Possible values are:
 *          "NoFontFamilies"      : Don't load any font families
 *          "DefaultFontFamilies" : A subset of available font families; currently the "web-safe" fonts (default).
 *          "AllFontFamilies"     : All local font families (not reccomended)
 *  - localFonts.extraFamilies: [string]
 *       Adds additional font families to be loaded at startup.
 *
 * @return Promise<instance: EmscriptenModule>
 *      The promise is resolved when the module has been instantiated and its main function has been
 *      called.
 *
 * @see https://github.com/DefinitelyTyped/DefinitelyTyped/blob/master/types/emscripten for
 *      EmscriptenModule
 */
async function qtLoad(config)
{
    const throwIfEnvUsedButNotExported = (instance, config) =>
    {
        const environment = config.qt.environment;
        if (!environment || Object.keys(environment).length === 0)
            return;
        const descriptor = Object.getOwnPropertyDescriptor(instance, 'ENV');
        const isEnvExported = typeof descriptor.value === 'object';
        if (!isEnvExported) {
            throw new Error('ENV must be exported if environment variables are passed, ' +
                            'add it to the QT_WASM_EXTRA_EXPORTED_METHODS CMake target property');
        }
    };

    if (typeof config !== 'object')
        throw new Error('config is required, expected an object');
    if (typeof config.qt !== 'object')
        throw new Error('config.qt is required, expected an object');
    if (typeof config.qt.entryFunction !== 'function')
        throw new Error('config.qt.entryFunction is required, expected a function');

    config.qt.qtdir ??= 'qt';
    config.qt.preload ??= [];

    config.qtContainerElements = config.qt.containerElements;
    delete config.qt.containerElements;
    config.qtFontDpi = config.qt.fontDpi;
    delete config.qt.fontDpi;

    // Make Emscripten not call main(); this gives us more control over
    // the startup sequence.
    const originalNoInitialRun = config.noInitialRun;
    const originalArguments = config.arguments;
    config.noInitialRun = true;

    // Used for rejecting a failed load's promise where emscripten itself does not allow it,
    // like in instantiateWasm below. This allows us to throw in case of a load error instead of
    // hanging on a promise to entry function, which emscripten unfortunately does.
    let circuitBreakerReject;
    const circuitBreaker = new Promise((_, reject) => { circuitBreakerReject = reject; });

    // If module async getter is present, use it so that module reuse is possible.
    if (config.qt.module) {
        config.instantiateWasm = async (imports, successCallback) =>
        {
            try {
                const module = await config.qt.module;
                successCallback(
                    await WebAssembly.instantiate(module, imports), module);
            } catch (e) {
                circuitBreakerReject(e);
            }
        }
    }
    const preloadFetchHelper = async (path) => {
        const response = await fetch(path);
        if (!response.ok)
            throw new Error("Could not fetch preload file: " + path);
        return response.json();
    }
    const filesToPreload = (await Promise.all(config.qt.preload.map(preloadFetchHelper))).flat();
    const qtPreRun = (instance) => {
        // Copy qt.environment to instance.ENV
        throwIfEnvUsedButNotExported(instance, config);
        for (const [name, value] of Object.entries(config.qt.environment ?? {}))
            instance.ENV[name] = value;

        // Preload files from qt.preload
        const makeDirs = (FS, filePath) => {
            const parts = filePath.split("/");
            let path = "/";
            for (let i = 0; i < parts.length - 1; ++i) {
                const part = parts[i];
                if (part == "")
                    continue;
                path += part + "/";
                try {
                    FS.mkdir(path);
                } catch (error) {
                    const EEXIST = 20;
                    if (error.errno != EEXIST)
                        throw error;
                }
            }
        }

        const extractFilenameAndDir = (path) => {
            const parts = path.split('/');
            const filename = parts.pop();
            const dir = parts.join('/');
            return {
                filename: filename,
                dir: dir
            };
        }
        const preloadFile = (file) => {
            makeDirs(instance.FS, file.destination);
            const source = file.source.replace('$QTDIR', config.qt.qtdir);
            const filenameAndDir = extractFilenameAndDir(file.destination);
            instance.FS.createPreloadedFile(filenameAndDir.dir, filenameAndDir.filename, source, true, true);
        }
        const isFsExported = typeof instance.FS === 'object';
        if (!isFsExported)
            throw new Error('FS must be exported if preload is used');
        filesToPreload.forEach(preloadFile);
    }

    if (!config.preRun)
        config.preRun = [];
    config.preRun.push(qtPreRun);

    const originalOnRuntimeInitialized = config.onRuntimeInitialized;
    config.onRuntimeInitialized = () => {
        originalOnRuntimeInitialized?.();
        config.qt.onLoaded?.();
    }

    const originalLocateFile = config.locateFile;
    config.locateFile = filename => {
        const originalLocatedFilename = originalLocateFile ? originalLocateFile(filename) : filename;
        if (originalLocatedFilename.startsWith('libQt6'))
            return `${config.qt.qtdir}/lib/${originalLocatedFilename}`;
        return originalLocatedFilename;
    }

    let onExitCalled = false;
    const originalOnExit = config.onExit;
    config.onExit = code => {
        originalOnExit?.();

        if (!onExitCalled) {
            onExitCalled = true;
            config.qt.onExit?.({
                code,
                crashed: false
            });
        }
    }

    const originalOnAbort = config.onAbort;
    config.onAbort = text =>
    {
        originalOnAbort?.();
        
        if (!onExitCalled) {
            onExitCalled = true;
            config.qt.onExit?.({
                text,
                crashed: true
            });
        }
    };

    // Call app/emscripten module entry function. It may either come from the emscripten
    // runtime script or be customized as needed.
    let instance;
    try {
        instance = await Promise.race(
            [circuitBreaker, config.qt.entryFunction(config)]);

        // Call main after creating the instance. We've opted into manually
        // calling main() by setting noInitialRun in the config. Thie Works around
        // issue where Emscripten suppresses all exceptions thrown during main.
        if (!originalNoInitialRun)
            instance.callMain(originalArguments);
    } catch (e) {
        // If this is the exception thrown by app.exec() then that is a normal
        // case and we suppress it.
        if (e == "unwind") // not much to go on
            return;

        if (!onExitCalled) {
            onExitCalled = true;
            config.qt.onExit?.({
                text: e.message,
                crashed: true
            });
        }
        throw e;
    }

    return instance;
}

export default qtLoad;