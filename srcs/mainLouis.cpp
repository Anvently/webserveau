#include <Header.hpp>

int	main()
{
	Header	hd;

	std::string	test = "";
	test += "POST /w/load.php?lang=fr&modules=ext.discussionTools.init.styles%7Cext.uls.interlanguage%7Cext.visualEditor.desktopArticleTarget.noscript%7Cext.wikimediaBadges%7Cext.wikimediamessages.styles%7Cmediawiki.widgets.styles%7Coojs-ui-core.icons%2Cstyles%7Coojs-ui.styles.indicators%7Cskins.vector.icons%2Cstyles%7Cskins.vector.search.codex.styles%7Cwikibase.client.init&only=styles&skin=vector-2022 HTTP/2.0\r\n";
	test += "Host: fr.wikipedia.org\r\n";
	test += "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:109.0) Gecko/20100101 Firefox/115.0\r\n";
	test += "Accept: text/css,*/*;q=0.1\r\n";
	test += "Content: first\r\n";
	test+= "Accept-Language: en-US,en;q=0.5\r\n";
	test+= "Content: second \r\n";
	test+="Void:\r\n";
	test += "\r\n";
	hd.parseInput(test);
	hd.formatHeaders();
	hd.printHeaders();
	hd.printRequest();
}
