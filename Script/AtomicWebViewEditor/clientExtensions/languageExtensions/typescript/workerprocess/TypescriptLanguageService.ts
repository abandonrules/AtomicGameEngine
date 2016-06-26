//
// Copyright (c) 2014-2016 THUNDERBEAST GAMES LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

// Based upon the TypeScript language services example at https://github.com/Microsoft/TypeScript/wiki/Using-the-Compiler-API#incremental-build-support-using-the-language-services

import * as ts from "../../../../modules/typescript";

/**
 * Abstraction over the file system
 */
export interface FileSystemInterface {
    /**
     * Deterimine if the particular file exists in the resources
     * @param  {string} filename
     * @return {boolean}
     */
    fileExists(filename: string): boolean;
    /**
     * Grab the contents of the file
     * @param  {string} filename
     * @return {string}
     */
    getFile(filename: string): string;
    /**
     * Write the contents to the file specified
     * @param  {string} filename
     * @param  {string} contents
     */
    writeFile(filename: string, contents: string);

    /**
     * Returns the current directory / root of the source tree
     * @return {string}
     */
    getCurrentDirectory(): string;
}

/**
 * Resource extension that handles compiling or transpling typescript on file save.
 */
export class TypescriptLanguageService {

    constructor(fs: FileSystemInterface, tsconfig: any) {
        this.fs = fs;
        this.setTsConfig(tsconfig);

        // Create the language service files
        this.documentRegistry = ts.createDocumentRegistry();
        this.createLanguageService(this.documentRegistry);
    }

    private fs: FileSystemInterface = null;
    private languageService: ts.LanguageService = null;
    private documentRegistry: ts.DocumentRegistry = null;

    private compilerOptions: ts.CompilerOptions = null;

    name: string = "TypescriptLanguageService";

    /**
     * Perform a full compile on save, or just transpile the current file
     * @type {boolean}
     */
    fullCompile: boolean = true;

    private projectFiles: string[] = [];
    private versionMap: ts.Map<{ version: number, snapshot?: ts.IScriptSnapshot }> = {};

    private createLanguageService(documentRegistry: ts.DocumentRegistry) {
        // Create the language service host to allow the LS to communicate with the host
        const servicesHost: ts.LanguageServiceHost = {
            getScriptFileNames: () => this.projectFiles,
            getScriptVersion: (fileName) => this.versionMap[fileName] && this.versionMap[fileName].version.toString(),
            getScriptSnapshot: (filename) => {
                const scriptVersion = this.versionMap[filename];

                // Grab the cached version
                if (scriptVersion) {
                    if (scriptVersion.snapshot) {
                        return scriptVersion.snapshot;
                    } else {
                        let sourceFile = this.documentRegistry.acquireDocument(filename, this.compilerOptions, ts.ScriptSnapshot.fromString(""), scriptVersion.version.toString());
                        return ts.ScriptSnapshot.fromString(sourceFile.text);
                    }
                }
            },
            getCurrentDirectory: () => this.fs.getCurrentDirectory(),
            getCompilationSettings: () => this.compilerOptions,
            getDefaultLibFileName: (options) => undefined
        };

        this.languageService = ts.createLanguageService(servicesHost, documentRegistry);
    }

    /**
     * Sets the tsconfig file and parses out the command line options
     * @param  {any} tsConfig
     * @return {[type]}
     */
    setTsConfig(tsConfig: any) {
        let cmdLine = ts.parseJsonConfigFileContent(tsConfig, undefined, undefined);
        this.compilerOptions = cmdLine.options;
    }

    /**
     * Adds a file to the internal project cache
     * @param  {string} filename the full path of the file to add
     * @param [{sring}] fileContents optional file contents.  If not provided, the filesystem object will be queried
     */
    addProjectFile(filename: string, fileContents?: string) {
        if (this.projectFiles.indexOf(filename) == -1) {
            console.log("Added project file: " + filename);
            this.versionMap[filename] = {
                version: 0,
                snapshot: ts.ScriptSnapshot.fromString(fileContents || this.fs.getFile(filename))
            };
            this.projectFiles.push(filename);
            this.documentRegistry.acquireDocument(
                filename,
                this.compilerOptions,
                this.versionMap[filename].snapshot,
                "0");
        }
    }

    /**
     * Updates the internal file representation.
     * @param  {string} filename name of the file
     * @param  {string} fileContents optional contents of the file.
     * @return {ts.SourceFile}
     */
    updateProjectFile(filename: string, fileContents: string): ts.SourceFile {
        this.versionMap[filename].version++;
        this.versionMap[filename].snapshot = ts.ScriptSnapshot.fromString(fileContents);

        return this.documentRegistry.updateDocument(
            filename,
            this.compilerOptions,
            this.versionMap[filename].snapshot,
            this.versionMap[filename].version.toString());
    }

    /**
     * Updates the internal file version number
     * @param  {string} filename name of the file
     * @return {ts.SourceFile}
     */
    updateProjectFileVersionNumber(filename: string): ts.SourceFile {
        this.versionMap[filename].version++;

        return this.documentRegistry.updateDocument(
            filename,
            this.compilerOptions,
            this.versionMap[filename].snapshot,
            this.versionMap[filename].version.toString());
    }
    /**
     * Returns the list of project files
     * @return {string[]}
     */
    getProjectFiles(): string[] {
        return this.projectFiles;
    }

    getPreEmitWarnings(filename: string) {
        let allDiagnostics = this.compileFile(filename);
        let results = [];

        allDiagnostics.forEach(diagnostic => {
            let lineChar = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
            let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
            results.push({
                row: lineChar.line,
                column: lineChar.character,
                text: message,
                type: diagnostic.category == 1 ? "error" : "warning"
            });
        });
        return results;
    }

    /**
     * Simply transpile the typescript file.  This is much faster and only checks for syntax errors
     * @param {string[]}           fileNames array of files to transpile
     */
    transpile(fileNames: string[]): void {
        fileNames.forEach((fileName) => {
            console.log(`${this.name}:  Transpiling ${fileName}`);
            let script = this.fs.getFile(fileName);
            let diagnostics: ts.Diagnostic[] = [];
            let result = ts.transpile(script, this.compilerOptions, fileName, diagnostics);
            if (diagnostics.length) {
                this.logErrors(diagnostics);
            }
            if (diagnostics.length == 0) {
                this.fs.writeFile(fileName.replace(".ts", ".js"), result);
            }
        });
    }

    /**
     * Converts the line and character offset to a position
     * @param  {ts.SourceFile} file
     * @param  {number} line
     * @param  {number} character
     * @return {number}
     */
    getPositionOfLineAndCharacter(file: ts.SourceFile, line: number, character: number): number {
        return ts.getPositionOfLineAndCharacter(file, line, character);
    }

    /**
     * Returns the completions available at the provided position
     * @param  {string} filename
     * @param  {number} position
     * @return {ts.CompletionInfo}
     */
    getCompletions(filename: string, pos: number): ts.CompletionInfo {
        return this.languageService.getCompletionsAtPosition(filename, pos);
    }

    /**
     * Returns details of this completion entry
     * @param  {string} filename
     * @param  {number} pos
     * @param  {string} entryname
     * @return {ts.CompletionEntryDetails}
     */
    getCompletionEntryDetails(filename: string, pos: number, entryname: string): ts.CompletionEntryDetails {
        return this.languageService.getCompletionEntryDetails(filename, pos, entryname);
    }

    /**
     * Compile the provided file to javascript with full type checking etc
     * @param  {string}  a list of file names to compile
     * @param  {function} optional callback which will be called for every file compiled and will provide any errors
     */
    compile(files: string[], progress?: (filename: string, errors: ts.Diagnostic[]) => void): ts.Diagnostic[] {
        let start = new Date().getTime();

        //Make sure we have these files in the project
        files.forEach((file) => {
            this.addProjectFile(file);
        });

        let errors: ts.Diagnostic[] = [];

        // if we have a 0 length for files, let's compile all files
        if (files.length == 0) {
            files = this.projectFiles;
        }

        files.forEach(filename => {
            let currentErrors = this.compileFile(filename);
            errors = errors.concat(currentErrors);
            if (progress) {
                progress(filename, currentErrors);
            }
        });

        console.log(`${this.name}: Compiling complete after ${new Date().getTime() - start} ms`);
        return errors;
    }

    /**
     * Delete a file from the project store
     * @param  {string} filepath
     */
    deleteProjectFile(filepath: string) {
        if (this.versionMap[filepath]) {
            delete this.versionMap[filepath];
        }
        let idx = this.projectFiles.indexOf(filepath);
        if (idx > -1) {
            console.log(`delete project file from ${filepath}`);
            this.projectFiles.splice(idx, 1);
        }
    }

    /**
     * rename a file in the project store
     * @param  {string} filepath the old path to the file
     * @param  {string} newpath the new path to the file
     */
    renameProjectFile(filepath: string, newpath: string): void {
        let oldFile = this.versionMap[filepath];
        if (oldFile) {
            console.log(`Rename project file from ${filepath} to ${newpath}`);
            delete this.versionMap[filepath];
            this.versionMap[newpath] = oldFile;
        }
        let idx = this.projectFiles.indexOf(filepath);
        if (idx > -1) {
            console.log(`Update project files array from ${filepath} to ${newpath}`);
            this.projectFiles[idx] = newpath;
        }
    }

    /**
     * clear out any caches, etc.
     */
    reset() {
        this.projectFiles = [];
        this.versionMap = {};
        //this.languageService = null;
    }

    /**
     * Compile an individual file
     * @param  {string} filename the file to compile
     * @return {[ts.Diagnostic]} a list of any errors
     */
    private compileFile(filename: string): ts.Diagnostic[] {
        console.log(`${this.name}: Compiling version ${this.versionMap[filename].version} of ${filename}`);
        //if (!filename.match("\.d\.ts$")) {
        try {
            return this.emitFile(filename);
        } catch (err) {
            console.log(`${this.name}: problem encountered compiling ${filename}: ${err}`);
            return [];
            // console.log(err.stack);
        }
        //}
    }

    /**
     * writes out a file from the compile process
     * @param  {string} filename [description]
     * @return {[ts.Diagnostic]} a list of any errors
     */
    private emitFile(filename: string): ts.Diagnostic[] {
        let output = this.languageService.getEmitOutput(filename);
        let allDiagnostics: ts.Diagnostic[] = [];
        if (output.emitSkipped) {
            console.log(`${this.name}: Failure Emitting ${filename}`);
            allDiagnostics = this.languageService.getCompilerOptionsDiagnostics()
                .concat(this.languageService.getSyntacticDiagnostics(filename))
                .concat(this.languageService.getSemanticDiagnostics(filename));
        }

        output.outputFiles.forEach(o => {
            this.fs.writeFile(o.name, o.text);
        });
        return allDiagnostics;
    }

    /**
     * Logs errors from the diagnostics returned from the compile/transpile process
     * @param {ts.Diagnostic} diagnostics information about the errors
     * @return {[type]}          [description]
     */
    private logErrors(diagnostics: ts.Diagnostic[]) {
        let msg = [];

        diagnostics.forEach(diagnostic => {
            let message = ts.flattenDiagnosticMessageText(diagnostic.messageText, "\n");
            if (diagnostic.file) {
                let d = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);
                let line = d.line;
                let character = d.character;

                // NOTE: Destructuring throws an error in chrome web view
                //let { line, character } = diagnostic.file.getLineAndCharacterOfPosition(diagnostic.start);

                msg.push(`${this.name}:  Error ${diagnostic.file.fileName} (${line + 1},${character + 1}): ${message}`);
            }
            else {
                msg.push(`${this.name}  Error: ${message}`);
            }
        });
        console.log(`TypeScript Errors:\n${msg.join("\n")}`);
        throw new Error(`TypeScript Errors:\n${msg.join("\n")}`);
    }

}
