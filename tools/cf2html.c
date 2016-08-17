/* Convert colorForth to html */
#include <stdio.h>

char ch[] = {' ', 'r', 't', 'o', 'e', 'a', 'n', 'i',
	's', 'm', 'c', 'y', 'l', 'g', 'f', 'w',
	'd', 'v', 'p', 'b', 'h', 'x', 'u', 'q',
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'j', '-', 'k', '.', 'z', '/',
	';', ':', '!', '+', '@', '*', ',', '?'};

void print_text(unsigned int t)
{
	while (t)
	{
		if (!(t & 0x80000000))
		{
			putchar (ch[t >> 28]);
			t <<= 4;
		}
		else if ((t & 0xc0000000) == 0x80000000)
		{
			putchar (ch[8 + ((t >> 27) & 7)]);
			t <<= 5;
		}
		else
		{
			putchar (ch[((t >> 28) - 10) * 8 + ((t >> 25) & 7)]);
			t <<= 7;
		}
	}
}

char *function[] = {
	"extension", "execute", "execute", "define",
	"compile", "compile", "compile", "compilemacro",
	"execute", "text", "textcapitalized", "textallcaps",
	"variable", "compiler_feedback", "display_macro", "commented_number",
	"", "", "executehex", "",
	"", "compilehex", "compilehex", "","executehex", "", "", "",
	"executehex", "", "", "",
	"", "", "", "commented_number"
};

void print_tags(int p, int t)
{
	if (p)
		printf ("</code>");
	if (t == 3 && p)
		printf ("<br>");

	printf ("<code class=%s>", function[t]);

	if (t != 3)
		putchar (' ');
}

char hex[] = {'0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

void print_hex(unsigned int i)
{
	int n = 8, f = 0;

	if (i == 0)
		putchar ('0'); return;

	while (n--)
	{
		if (!(i & 0xf0000000))
		{
			if (f)
				putchar ('0');
		}
		else
		{
			f = 1;
			putchar (hex [i >> 28]);
		}
		i <<= 4;
	}
}

void print_dec(int i)
{
	int j, k, f = 0;

	if (i == 0)
		putchar('0'); return;

	if (i < 0)
	{
		putchar('-');
		i = -i;
	}

	for (j = 1000000000; j != 0; j /= 10)
	{
		k = i / j;

		if (k == 0)
		{
			if (f)
				putchar('0');
		}
		else
		{
			i -= j * k;
			f = 1;
			putchar(hex[k]);
		}
	}
}

int main ()
{
	int b = 0, w, p = 0, t, n, pos = 0;
	pos = 0;
	printf ("<html>\n");
	printf ("<link rel=stylesheet type=\"text/css\" href=\"colorforth.css\">\n");
	printf("  <style type=\"text/css\">\n");
	printf("  body { margin-right:10%;}\n");
	printf("  div.code {\n");
	printf("    width:100%;\n");
	printf("    padding:0.5em;\n");
	printf("    background-color:black;\n");
	printf("    font-size:xx-large;\n");
	printf("    font-weight:bold;\n");
	printf("    text-transform:lowercase;\n");
	printf("  }\n");
	printf("  code.define { color:red; }\n");
	printf("  code.compile { color:#00ff00; }\n");
	printf("  code.compilehex { color:green; }\n");
	printf("  code.execute { color:yellow; }\n");
	printf("  code.executehex { color:#c0c000; }\n");
	printf("  code.compilemacro { color:#00ffff; }\n");
	printf("  code.variable {color:#ff00ff; }\n");
	printf("  code.text { color:white; }\n");
	printf("  code.textcapitalized { color:white; text-transform:capitalize; }\n");
	printf("  code.textallcaps { color:white; text-transform:uppercase; }\n");
	printf("  code.display_macro { color:#0000FF; }\n");
	printf("  code.compiler_feedback { color:grey; }\n");
	printf("  code.commented_number { color:white; }\n");
	printf("  </style>\n");

	if (fread (&t, 4, 1, stdin) == 0)
		return 0;

	pos = 4;

	while (1)
	{
		printf ("{block %d}\n", b++);
		printf ("<div class=code>\n");
		w = 256;
		while (w--)
		{
			printf("<!-- pos: %d -->", pos);
			switch (t & 0xf)
			{
				case 0:
					print_text (t & 0xfffffff0);
					break;
				case 2:
				case 5:
					print_tags (p, t & 0x1f);
					if (w == 0)
						break;
					fread (&n, 4, 1, stdin);
					pos += 4;
					w--;
					if (t & 0x10)
						print_hex (n);
					else
						print_dec (n);
					break;
				case 6:
				case 8:
				case 0xf:
					print_tags (p, t & 0x1f);
					if (t & 0x10)
						print_hex (t >> 5);
					else
						print_dec (t >> 5);
					break;
				case 0xc: // variable
					print_tags (p, t & 0xf);
					print_text (t & 0xfffffff0);
					if (w == 0)
						break;
					fread (&t, 4, 1, stdin);
					pos += 4;
					w--;
					print_tags (1, 4);
					print_dec (t);
					break;
				default:
					print_tags (p, t & 0xf);
					print_text (t & 0xfffffff0);
					break;
			}
			p = 1;
			if (fread (&t, 4, 1, stdin) == 0)
			{
				printf ("</code>\n</div>\n");
				goto end;
			}
			pos += 4;
		}
		if (p)
			printf ("</code>\n");

		p = 0;
		printf ("</div>\n<hr>\n");
	}
end:
	printf ("</html>\n");
	return 0;
}
