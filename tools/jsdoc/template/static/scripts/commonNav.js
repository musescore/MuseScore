if (typeof fetch === 'function') {
  const init = () => {
    if (typeof scrollToNavItem !== 'function') return false
    scrollToNavItem()
    // hideAllButCurrent not always loaded
    if (typeof hideAllButCurrent === 'function') hideAllButCurrent()
    return true
  }
  fetch('./nav.inc.html')
    .then(response => response.ok ? response.text() : `${response.url} => ${response.status} ${response.statusText}`)
    .then(body => {
      document.querySelector('nav').innerHTML += body
      // nav.js should be quicker to load than nav.inc.html, a fallback just in case
      return init()
    })
    .then(done => {
      if (done) return
      let i = 0
      ;(function waitUntilNavJs () {
        if (init()) return
        if (i++ < 100) return setTimeout(waitUntilNavJs, 300)
        console.error(Error('nav.js not loaded after 30s waiting for it'))
      })()
    })
    .catch(error => console.error(error))
} else {
  console.error(Error('Browser too old to display commonNav (remove commonNav docdash option)'))
}
