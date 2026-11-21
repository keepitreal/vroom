import type { BaseLayoutProps } from 'fumadocs-ui/layouts/shared';
import { GITHUB_URL } from './site';

export function baseOptions(): BaseLayoutProps {
  return {
    nav: {
      title: <span className="fd-wordmark">VROOM</span>,
      url: '/',
    },
    githubUrl: GITHUB_URL,
    links: [{ text: 'Home', url: '/', active: 'url' }],
  };
}
