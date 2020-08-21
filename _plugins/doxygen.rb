require 'kramdown'

module Jekyll
  class DoxyPageGenerator < Generator
    safe true

    def generate(site)
      
        site.pages << DoxyPage.new(site, site.source, "test", "api1")      
        #site.pages << Page.new(site, site.source, 'test', 'apitest')      
    end
  end

  # A Page subclass used in the `CategoryPageGenerator`
  class DoxyPage < PageWithoutAFile
    def initialize(site, base, dir, name)
      @site = site
      @base = base
      @dir  = dir
      @name = name + '.html'

      #self.read_yaml(File.join(base, '_layouts'), 'post')


      @markup = "
# test

## Structures
<ul>
{% assign structs = site.data.doc.xml.index.doxygenindex.compound | where: 'kind', 'struct' %}
{% for struct in structs %}
    <li>
        {{struct.name}}
        <ul>
        {%for mb in struct.member%}
            <li>{{ mb.name }}</li>
        {% endfor %}
        </ul>
    </li>
{% endfor %}
</ul>

"    

    @content = Kramdown::Document.new(@markup).to_html

    self.process(@name)

      #self.data['category'] = name

      #category_title_prefix = site.config['category_title_prefix'] || 'Category: '
      self.data = Hash.new()
      self.data['title'] = self.data
      self.data['layout'] = 'default'
    end
  end
end