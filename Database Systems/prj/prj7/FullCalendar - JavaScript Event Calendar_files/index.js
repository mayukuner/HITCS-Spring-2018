$(function() {

  var todayDate = moment().startOf('day');
  var YM = todayDate.format('YYYY-MM');
  var YESTERDAY = todayDate.clone().subtract(1, 'day').format('YYYY-MM-DD');
  var TODAY = todayDate.format('YYYY-MM-DD');
  var TOMORROW = todayDate.clone().add(1, 'day').format('YYYY-MM-DD');

  $('#calendar').fullCalendar({
    header: {
      left: 'prev,next today',
      center: 'title',
      right: 'month,agendaWeek,agendaDay,listWeek'
    },
    editable: true,
    eventLimit: true, // allow "more" link when too many events
    navLinks: true,
    events: [
      {
        title: '2nd Interview - Alibaba',
        start: YM + '-01'
      },
      {
        title: '1st Interview - Google',
        start: YM + '-07',
        end: YM + '-07'
      },
      {
        title: '3rd Interview - Tencent',
        start: TODAY + 'T12:00:00'
      },
      {
        title: '2nd Interview - Google',
        start: TODAY + 'T17:30:00'
      },
      {
        title: '3nd Interview - Alibaba',
        start: YM + '-28'
      }
    ]
  });
});
