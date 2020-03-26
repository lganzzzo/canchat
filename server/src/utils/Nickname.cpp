/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "Nickname.hpp"
#include "oatpp/core/data/stream/BufferStream.hpp"

const char* const Nickname::ADJECTIVES[] = {
  "Attractive",
  "Bald",
  "Beautiful",
  "Chubby",
  "Clean",
  "Dazzling",
  "Drab",
  "Elegant",
  "Fancy",
  "Fit",
  "Flabby",
  "Glamorous",
  "Gorgeous",
  "Handsome",
  "Long",
  "Magnificent",
  "Muscular",
  "Plain",
  "Plump",
  "Quaint",
  "Scruffy",
  "Shapely",
  "Short",
  "Skinny",
  "Stocky",
  "Ugly",
  "Unkempt",
  "Unsightly",
  "Ashy",
  "Black",
  "Blue",
  "Gray",
  "Green",
  "Icy",
  "Lemon",
  "Mango",
  "Orange",
  "Purple",
  "Red",
  "Salmon",
  "White",
  "Yellow",
  "Aggressive",
  "Agreeable",
  "Ambitious",
  "Brave",
  "Calm",
  "Delightful",
  "Eager",
  "Faithful",
  "Gentle",
  "Happy",
  "Jolly",
  "Kindv",
  "Lively",
  "Nice",
  "Obedient",
  "Polite",
  "Proud",
  "Silly",
  "Thankful",
  "Victorious",
  "Witty",
  "Wonderful",
  "Zealous",
  "Broad",
  "Chubby",
  "Crooked",
  "Curved",
  "Deep",
  "Flat",
  "High",
  "Hollow",
  "Low",
  "Narrow",
  "Refined",
  "Round",
  "Shallow",
  "Skinny",
  "Square",
  "Steep",
  "Straight",
  "Wide",
  "Big",
  "Colossal",
  "Fat",
  "Gigantic",
  "Great",
  "Huge",
  "Immense",
  "Large",
  "Little",
  "Mammoth",
  "Massive",
  "Microscopic",
  "Miniature",
  "Petite",
  "Puny",
  "Scrawny",
  "Short",
  "Small",
  "Tall",
  "Teeny",
  "Tiny"
};

const char* Nickname::NOUNS[] = {
  "Area",
  "Book",
  "Business",
  "Case",
  "Child",
  "Company",
  "Country",
  "Day",
  "Eye",
  "Fact",
  "Family",
  "Government",
  "Group",
  "Hand",
  "Home",
  "Job",
  "Life",
  "Lot",
  "Man",
  "Money",
  "Month",
  "Mother",
  "Mr",
  "Night",
  "Number",
  "Part",
  "People",
  "Place",
  "Point",
  "Problem",
  "Program",
  "Question",
  "Right",
  "Room",
  "School",
  "State",
  "Story",
  "Student",
  "Study",
  "System",
  "Thing",
  "Time",
  "Water",
  "Way",
  "Week",
  "Woman",
  "Word",
  "Work",
  "World",
  "Year"
};

thread_local std::mt19937 Nickname::RANDOM_GENERATOR(std::random_device{}());

oatpp::String Nickname::random() {

  std::uniform_int_distribution<size_t> distroA(0, ADJECTIVES_SIZE);
  std::uniform_int_distribution<size_t> distroN(0, NOUNS_SIZE);

  oatpp::data::stream::BufferOutputStream stream;
  stream << ADJECTIVES[distroA(RANDOM_GENERATOR)] << " " << NOUNS[distroN(RANDOM_GENERATOR)];

  return stream.toString();

}