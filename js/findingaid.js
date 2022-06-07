document.addEventListener('DOMContentLoaded', function () {
  // anchor the fieldset title against the of the containing wrapper div
  // this allows a user to permalink to an arbitrary component depth on the title
  let wrappers = document.querySelectorAll('div.ead div.fieldset-wrapper[id]');
  if (wrappers !== null) {
    for (let index = 0; index < wrappers.length; ++index) {
      let title = wrappers[index].previousSibling.querySelector('span.fieldset-legend a.fieldset-title');
      if (title !== null) {
        title.setAttribute('href', '#' + wrappers[index].getAttribute('id'));
      }
    }
  }
  // when clicking on a link within a fieldset, provide a reference in the browser history back to the anchor
  // this allows a user to navigate "back" to the original position in the finding aid
  let links = document.querySelectorAll('div.ead div.fieldset-wrapper[id] a');
  if (links !== null) {
    for (let index = 0; index < links.length; ++index) {
      let target = links[index].closest('div.fieldset-wrapper[id]');
      if (target !== null) {
        links[index].onclick = function () {
          history.pushState({}, '', '#' + target.getAttribute('id'));
        }
      }
    }
  }
}, false);
