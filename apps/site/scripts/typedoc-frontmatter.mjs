// Local TypeDoc plugin: gives each generated page the frontmatter Fumadocs needs.
import { MarkdownPageEvent } from 'typedoc-plugin-markdown';

function summaryOf(model) {
  const comment = model.comment ?? model.signatures?.[0]?.comment;
  const text = (comment?.summary ?? [])
    .map((part) => part.text)
    .join('')
    .replace(/\{@link\s+([^}|\s]+)[^}]*\}/g, '$1')
    .replace(/[`*_]/g, '')
    .replace(/\s+/g, ' ')
    .trim();
  const sentence = text.match(/^.*?[.!?](\s|$)/)?.[0].trim() ?? text;
  return sentence.length > 200 ? `${sentence.slice(0, 197)}...` : sentence;
}

export function load(app) {
  app.renderer.on(MarkdownPageEvent.END, (page) => {
    const isIndex = page.url === 'index.md';
    const title = isIndex ? 'Overview' : page.model.name;
    const description = isIndex
      ? 'Every type and function exported by react-native-vroom-chart and @vroomchart/react.'
      : summaryOf(page.model);
    const head = ['---', `title: ${JSON.stringify(title)}`];
    if (description) head.push(`description: ${JSON.stringify(description)}`);
    head.push('---', '', '');
    page.contents = head.join('\n') + page.contents;
  });
}
