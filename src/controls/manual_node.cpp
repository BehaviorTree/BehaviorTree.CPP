/* Copyright (C) 2015-2018 Michele Colledanchise -  All Rights Reserved
 * Copyright (C) 2018-2020 Davide Faconti, Eurecat -  All Rights Reserved
*
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
*   to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
*   and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "behaviortree_cpp/controls/manual_node.h"
#include "behaviortree_cpp/action_node.h"
#include <ncurses.h>

namespace BT
{
ManualSelectorNode::ManualSelectorNode(const std::string& name, const NodeConfig& config)
  : ControlNode::ControlNode(name, config)
  , running_child_idx_(-1)
  , previously_executed_idx_(-1)
{
  setRegistrationID("ManualSelector");
}

void ManualSelectorNode::halt()
{
  if(running_child_idx_ >= 0)
  {
    haltChild(size_t(running_child_idx_));
  }
  running_child_idx_ = -1;
  ControlNode::halt();
}

NodeStatus ManualSelectorNode::tick()
{
  const size_t children_count = children_nodes_.size();

  if(children_count == 0)
  {
    return selectStatus();
  }

  bool repeat_last = false;
  getInput(REPEAT_LAST_SELECTION, repeat_last);

  int idx = 0;

  if(repeat_last && previously_executed_idx_ >= 0)
  {
    idx = previously_executed_idx_;
  }
  else
  {
    setStatus(NodeStatus::RUNNING);
    idx = selectChild();
    previously_executed_idx_ = idx;

    if(idx == NUM_SUCCESS)
    {
      return NodeStatus::SUCCESS;
    }
    if(idx == NUM_FAILURE)
    {
      return NodeStatus::FAILURE;
    }
    if(idx == NUM_RUNNING)
    {
      return NodeStatus::RUNNING;
    }
  }

  NodeStatus ret = children_nodes_[idx]->executeTick();
  if(ret == NodeStatus::RUNNING)
  {
    running_child_idx_ = idx;
  }
  return ret;
}

NodeStatus ManualSelectorNode::selectStatus() const
{
  WINDOW* win;
  initscr();
  cbreak();

  win = newwin(6, 70, 1, 1);  // create a new window

  mvwprintw(win, 0, 0, "No children.");
  mvwprintw(win, 1, 0, "Press: S to return SUCCESSFUL,");
  mvwprintw(win, 2, 0, "       F to return FAILURE, or");
  mvwprintw(win, 3, 0, "       R to return RUNNING.");

  wrefresh(win);      // update the terminal screen
  noecho();           // disable echoing of characters on the screen
  keypad(win, TRUE);  // enable keyboard input for the window.
  curs_set(0);        // hide the default screen cursor.

  int ch = 0;
  NodeStatus ret;
  while(1)
  {
    if(ch == 's' || ch == 'S')
    {
      ret = NodeStatus::SUCCESS;
      break;
    }
    else if(ch == 'f' || ch == 'F')
    {
      ret = NodeStatus::FAILURE;
      break;
    }
    else if(ch == 'r' || ch == 'R')
    {
      ret = NodeStatus::RUNNING;
      break;
    }
    ch = wgetch(win);
  }
  werase(win);
  wrefresh(win);
  delwin(win);
  endwin();
  return ret;
}

uint8_t ManualSelectorNode::selectChild() const
{
  const size_t children_count = children_nodes_.size();

  std::vector<std::string> list;
  list.reserve(children_count);
  for(const auto& child : children_nodes_)
  {
    list.push_back(child->name());
  }

  size_t width = 10;
  for(const auto& str : list)
  {
    width = std::max(width, str.size() + 2);
  }

  WINDOW* win;
  initscr();
  cbreak();

  win = newwin(children_count + 6, 70, 1, 1);  // create a new window

  mvwprintw(win, 0, 0, "Use UP/DOWN arrow to select the child, Enter to confirm.");
  mvwprintw(win, 1, 0, "Press: S to skip and return SUCCESSFUL,");
  mvwprintw(win, 2, 0, "       F to skip and return FAILURE, or");
  mvwprintw(win, 3, 0, "       R to skip and return RUNNING.");

  // now print all the menu items and highlight the first one
  for(size_t i = 0; i < list.size(); i++)
  {
    mvwprintw(win, i + 5, 0, "%2ld. %s", i + 1, list[i].c_str());
  }

  wrefresh(win);      // update the terminal screen
  noecho();           // disable echoing of characters on the screen
  keypad(win, TRUE);  // enable keyboard input for the window.
  curs_set(0);        // hide the default screen cursor.

  uint8_t row = 0;
  int ch = 0;
  while(1)
  {
    // right pad with spaces to make the items appear with even width.
    wattroff(win, A_STANDOUT);
    mvwprintw(win, row + 5, 4, "%s", list[row].c_str());
    // use a variable to increment or decrement the value based on the input.
    if(ch == KEY_DOWN)
    {
      row = (row == children_count - 1) ? 0 : row + 1;
    }
    else if(ch == KEY_UP)
    {
      row = (row == 0) ? (children_count - 1) : row - 1;
    }
    else if(ch == KEY_ENTER || ch == 10)
    {
      break;
    }
    else if(ch == 's' || ch == 'S')
    {
      row = NUM_SUCCESS;
      break;
    }
    else if(ch == 'f' || ch == 'F')
    {
      row = NUM_FAILURE;
      break;
    }
    else if(ch == 'r' || ch == 'R')
    {
      row = NUM_RUNNING;
      break;
    }

    // now highlight the next item in the list.
    wattron(win, A_STANDOUT);
    mvwprintw(win, row + 5, 4, "%s", list[row].c_str());
    ch = wgetch(win);
  }

  werase(win);
  wrefresh(win);
  delwin(win);
  endwin();
  return row;
}

}  // namespace BT
