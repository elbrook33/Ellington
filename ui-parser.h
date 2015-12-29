#ifndef UI_PARSER_H
#define UI_PARSER_H

#include "helpers.h"
#include <string.h>
#include <stdbool.h>

void uiParse(xWindow window, const char* markup,
	bool (*action)(xWindow, const char*, const char*, float, float, float*, int*, void*),
	void* data)
{
	if(!markup || !*markup) { return; }
	
	// Set up drawing context
	float x = window.canvas.left, y = window.canvas.top;
	nvgFontSize(window.canvas.nano, window.canvas.fontSize);
	nvgFontFace(window.canvas.nano, "normal");
	nvgFillColor(window.canvas.nano, window.canvas.fgColour);
	
	float spaceWidth =
		nvgTextBounds(window.canvas.nano, 0, 0, "M M", NULL, NULL)
		- nvgTextBounds(window.canvas.nano, 0, 0, "MM", NULL, NULL);
	
	// Traverse through pars, then tabs, then words
	char *pars = strdup(markup);
	char *par, *parProgress;
	
	int parIndex = 0;
	
	for(	par = strtok_r(pars, "\n", &parProgress);
		par != NULL;
		par = strtok_r(NULL, "\n", &parProgress))
	{
		parIndex += 1;
		
		char *tabs = par;
		char *tab, *tabProgress;
		
		// Tabs are effectively tables
		int tabIndex = 0;
		int tabCount = strcount(tabs, '\t') + 1;
		float tabWidth = (window.canvas.right - window.canvas.left) / (float)tabCount;
		
		int tabAlignment;
		
		for(	tab = strtok_r(tabs, "\t", &tabProgress);
			tab != NULL;
			tab = strtok_r(NULL, "\t", &tabProgress))
		{
			x = (float)tabIndex * tabWidth + window.canvas.left;
			tabIndex += 1;
			
			// Handle left, centre and right alignment
			switch(*tab)
			{
				case '>':
					tab += 1;
					int lastIndex = strlen(tab) - 1;					
					switch(tab[lastIndex])
					{
						case '>':
							tab[lastIndex] = '\0';
							tabAlignment = 1;
							nvgTextAlign(window.canvas.nano, NVG_ALIGN_RIGHT|NVG_ALIGN_TOP);
							x += tabWidth;
							break;
						case '<':
							tab[lastIndex] = '\0';
							tabAlignment = 0;
							nvgTextAlign(window.canvas.nano, NVG_ALIGN_CENTER|NVG_ALIGN_TOP);
							x += tabWidth / 2.0;
							break;
					}
					break;
				default:
					tabAlignment = -1;
					nvgTextAlign(window.canvas.nano, NVG_ALIGN_LEFT|NVG_ALIGN_TOP);
			}
			
			// Words may need to eventually be traversed twice for centre and right alignments
			char *words = tab;
			char *word, *wordProgress;
			
			int wordIndex = 0;

			for(	word = strtok_r(words, " ", &wordProgress);
				word != NULL;
				word = strtok_r(NULL, " ", &wordProgress))
			{
				wordIndex += 1;
				
				// Handle bold and italics
				switch(*word)
				{
					case '\\':
						word += 1;
						break;
					case '_':
						nvgFontFace(window.canvas.nano, "italic");
						word += 1;
						break;
					case '*':
						nvgFontFace(window.canvas.nano, "bold");
						word += 1;
						break;
				}
				
				// Handle trailing stylings
				int lastIndex = strlen(word) - 1;
				char lastLetter = '\0';
				switch(word[lastIndex])
				{
					case '\\':
						word[lastIndex] = '\0';
						break;
					case '_':
					case '*':
						word[lastIndex] = '\0';
						lastLetter = '_';
						break;
				}
				
				// Send word to "action"
				float bounds[4];
				float textWidth = nvgTextBounds(window.canvas.nano, x, y, word, NULL, bounds)
					+ spaceWidth;
				
				int indices[3] = { parIndex, tabIndex, wordIndex };

				bool keepGoing =
					action(window, word, NULL, x, y, bounds, indices, data);
				
				if(!keepGoing)
					{ free(pars); return; }
				
				// Advance text entry position
				switch(tabAlignment)
				{
					case -1: x += textWidth; break;
					case 1: x -= textWidth; break;
				}
				
				// Post-word processing
				switch(lastLetter)
				{
					case '_':
					case '*':
						nvgFontFace(window.canvas.nano, "normal");
						break;
				}
			}
		}
	}
	free(pars);
}

#endif
