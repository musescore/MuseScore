/* Copyright 2014, Google Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

 * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
 * Neither the name of Google Inc. nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*
Tool upload_system_symbols generates and uploads Breakpad symbol files for OS X system libraries.

This tool shells out to the dump_syms and symupload Breakpad tools. In its default mode, this
will find all dynamic libraries on the system, run dump_syms to create the Breakpad symbol files,
and then upload them to Google's crash infrastructure.

The tool can also be used to only dump libraries or upload from a directory. See -help for more
information.

Both i386 and x86_64 architectures will be dumped and uploaded.
*/
package main

import (
	"flag"
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"os/exec"
	"path"
	"regexp"
	"strings"
	"sync"
	"time"
)

var (
	breakpadTools  = flag.String("breakpad-tools", "out/Release/", "Path to the Breakpad tools directory, containing dump_syms and symupload.")
	uploadOnlyPath = flag.String("upload-from", "", "Upload a directory of symbol files that has been dumped independently.")
	dumpOnlyPath   = flag.String("dump-to", "", "Dump the symbols to the specified directory, but do not upload them.")
	systemRoot     = flag.String("system-root", "", "Path to the root of the Mac OS X system whose symbols will be dumped.")
)

var (
	// pathsToScan are the subpaths in the systemRoot that should be scanned for shared libraries.
	pathsToScan = []string{
		"/Library/QuickTime",
		"/System/Library/Components",
		"/System/Library/Frameworks",
		"/System/Library/PrivateFrameworks",
		"/usr/lib",
	}

	// uploadServers are the list of servers to which symbols should be uploaded.
	uploadServers = []string{
		"https://clients2.google.com/cr/symbol",
		"https://clients2.google.com/cr/staging_symbol",
	}

	// blacklistRegexps match paths that should be excluded from dumping.
	blacklistRegexps = []*regexp.Regexp{
		regexp.MustCompile(`/System/Library/Frameworks/Python\.framework/`),
		regexp.MustCompile(`/System/Library/Frameworks/Ruby\.framework/`),
		regexp.MustCompile(`_profile\.dylib$`),
		regexp.MustCompile(`_debug\.dylib$`),
		regexp.MustCompile(`\.a$`),
		regexp.MustCompile(`\.dat$`),
	}
)

func main() {
	flag.Parse()
	log.SetFlags(0)

	var uq *UploadQueue

	if *uploadOnlyPath != "" {
		// -upload-from specified, so handle that case early.
		uq = StartUploadQueue()
		uploadFromDirectory(*uploadOnlyPath, uq)
		uq.Wait()
		return
	}

	if *systemRoot == "" {
		log.Fatal("Need a -system-root to dump symbols for")
	}

	if *dumpOnlyPath != "" {
		// -dump-to specified, so make sure that the path is a directory.
		if fi, err := os.Stat(*dumpOnlyPath); err != nil {
			log.Fatal("-dump-to location: %v", err)
		} else if !fi.IsDir() {
			log.Fatal("-dump-to location is not a directory")
		}
	}

	dumpPath := *dumpOnlyPath
	if *dumpOnlyPath == "" {
		// If -dump-to was not specified, then run the upload pipeline and create
		// a temporary dump output directory.
		uq = StartUploadQueue()

		if p, err := ioutil.TempDir("", "upload_system_symbols"); err != nil {
			log.Fatal("Failed to create temporary directory: %v", err)
		} else {
			dumpPath = p
			defer os.RemoveAll(p)
		}
	}

	dq := StartDumpQueue(*systemRoot, dumpPath, uq)
	dq.Wait()
	if uq != nil {
		uq.Wait()
	}
}

type WorkerPool struct {
	wg sync.WaitGroup
}

// StartWorkerPool will launch numWorkers goroutines all running workerFunc.
// When workerFunc exits, the goroutine will terminate.
func StartWorkerPool(numWorkers int, workerFunc func()) *WorkerPool {
	p := new(WorkerPool)
	for i := 0; i < numWorkers; i++ {
		p.wg.Add(1)
		go func() {
			workerFunc()
			p.wg.Done()
		}()
	}
	return p
}

// Wait for all the workers in the pool to complete the workerFunc.
func (p *WorkerPool) Wait() {
	p.wg.Wait()
}

type UploadQueue struct {
	*WorkerPool
	queue chan string
}

// StartUploadQueue creates a new worker pool and queue, to which paths to
// Breakpad symbol files may be sent for uploading.
func StartUploadQueue() *UploadQueue {
	uq := &UploadQueue{
		queue: make(chan string, 10),
	}
	uq.WorkerPool = StartWorkerPool(5, uq.worker)
	return uq
}

// Upload enqueues the contents of filepath to be uploaded.
func (uq *UploadQueue) Upload(filepath string) {
	uq.queue <- filepath
}

// Done tells the queue that no more files need to be uploaded. This must be
// called before WorkerPool.Wait.
func (uq *UploadQueue) Done() {
	close(uq.queue)
}

func (uq *UploadQueue) worker() {
	symUpload := path.Join(*breakpadTools, "symupload")

	for symfile := range uq.queue {
		for _, server := range uploadServers {
			for i := 0; i < 3; i++ { // Give each upload 3 attempts to succeed.
				cmd := exec.Command(symUpload, symfile, server)
				if output, err := cmd.Output(); err == nil {
					// Success. No retry needed.
					fmt.Printf("Uploaded %s to %s\n", symfile, server)
					break
				} else {
					log.Printf("Error running symupload(%s, %s), attempt %d: %v: %s\n", symfile, server, i, err, output)
					time.Sleep(1 * time.Second)
				}
			}
		}
	}
}

type DumpQueue struct {
	*WorkerPool
	dumpPath string
	queue    chan dumpRequest
	uq       *UploadQueue
}

type dumpRequest struct {
	path string
	arch string
}

// StartDumpQueue creates a new worker pool to find all the Mach-O libraries in
// root and dump their symbols to dumpPath. If an UploadQueue is passed, the
// path to the symbol file will be enqueued there, too.
func StartDumpQueue(root, dumpPath string, uq *UploadQueue) *DumpQueue {
	dq := &DumpQueue{
		dumpPath: dumpPath,
		queue:    make(chan dumpRequest),
		uq:       uq,
	}
	dq.WorkerPool = StartWorkerPool(12, dq.worker)

	findLibsInRoot(root, dq)

	return dq
}

// DumpSymbols enqueues the filepath to have its symbols dumped in the specified
// architecture.
func (dq *DumpQueue) DumpSymbols(filepath string, arch string) {
	dq.queue <- dumpRequest{
		path: filepath,
		arch: arch,
	}
}

func (dq *DumpQueue) Wait() {
	dq.WorkerPool.Wait()
	if dq.uq != nil {
		dq.uq.Done()
	}
}

func (dq *DumpQueue) done() {
	close(dq.queue)
}

func (dq *DumpQueue) worker() {
	dumpSyms := path.Join(*breakpadTools, "dump_syms")

	for req := range dq.queue {
		filebase := path.Join(dq.dumpPath, strings.Replace(req.path, "/", "_", -1))
		symfile := fmt.Sprintf("%s_%s.sym", filebase, req.arch)
		f, err := os.Create(symfile)
		if err != nil {
			log.Fatal("Error creating symbol file:", err)
		}

		cmd := exec.Command(dumpSyms, "-a", req.arch, req.path)
		cmd.Stdout = f
		err = cmd.Run()
		f.Close()

		if err != nil {
			os.Remove(symfile)
			log.Printf("Error running dump_syms(%s, %s): %v\n", req.arch, req.path, err)
		} else if dq.uq != nil {
			dq.uq.Upload(symfile)
		}
	}
}

// uploadFromDirectory handles the upload-only case and merely uploads all files in
// a directory.
func uploadFromDirectory(directory string, uq *UploadQueue) {
	d, err := os.Open(directory)
	if err != nil {
		log.Fatal("Could not open directory to upload: %v", err)
	}
	defer d.Close()

	entries, err := d.Readdirnames(0)
	if err != nil {
		log.Fatal("Could not read directory: %v", err)
	}

	for _, entry := range entries {
		uq.Upload(path.Join(directory, entry))
	}

	uq.Done()
}

// findQueue is an implementation detail of the DumpQueue that finds all the
// Mach-O files and their architectures.
type findQueue struct {
	*WorkerPool
	queue chan string
	dq    *DumpQueue
}

// findLibsInRoot looks in all the pathsToScan in the root and manages the
// interaction between findQueue and DumpQueue.
func findLibsInRoot(root string, dq *DumpQueue) {
	fq := &findQueue{
		queue: make(chan string, 10),
		dq:    dq,
	}
	fq.WorkerPool = StartWorkerPool(12, fq.worker)

	for _, p := range pathsToScan {
		fq.findLibsInPath(path.Join(root, p))
	}

	close(fq.queue)
	fq.Wait()
	dq.done()
}

// findLibsInPath recursively walks the directory tree, sending file paths to
// test for being Mach-O to the findQueue.
func (fq *findQueue) findLibsInPath(loc string) {
	d, err := os.Open(loc)
	if err != nil {
		log.Fatal("Could not open %s: %v", loc, err)
	}
	defer d.Close()

	for {
		fis, err := d.Readdir(100)
		if err != nil && err != io.EOF {
			log.Fatal("Error reading directory %s: %v", loc, err)
		}

		for _, fi := range fis {
			fp := path.Join(loc, fi.Name())
			if fi.IsDir() {
				fq.findLibsInPath(fp)
				continue
			} else if fi.Mode()&os.ModeSymlink != 0 {
				continue
			}

			// Test the blacklist in the worker to not slow down this main loop.

			fq.queue <- fp
		}

		if err == io.EOF {
			break
		}
	}
}

func (fq *findQueue) worker() {
	for fp := range fq.queue {
		blacklisted := false
		for _, re := range blacklistRegexps {
			blacklisted = blacklisted || re.MatchString(fp)
		}
		if blacklisted {
			continue
		}

		imageinfos, err := GetMachOImageInfo(fp)
		if err != nil && err != ErrNotMachO {
			log.Printf("%s: %v", fp, err)
			continue
		}

		for _, imageinfo := range imageinfos {
			if imageinfo.Type == MachODylib || imageinfo.Type == MachOBundle {

				fq.dq.DumpSymbols(fp, imageinfo.Arch)
			}
		}
	}
}
