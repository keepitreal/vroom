import { createRoot } from 'react-dom/client';
import { App } from './App';

// Reports to the console whether this page's CSP blocks eval — see
// public/csp-probe.js. Must be an external script to get an honest answer.
document.head.appendChild(
  Object.assign(document.createElement('script'), { src: '/csp-probe.js' }),
);

createRoot(document.getElementById('root')!).render(<App />);
