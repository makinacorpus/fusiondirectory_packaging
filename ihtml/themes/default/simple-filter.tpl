<div class="contentboxh">
 <p class="contentboxh">
  {t}Filter{/t}
 </p>
</div>

<div class="contentboxb">
  {foreach from=$objectFilters item="ofilter"}
    {${$ofilter.id}}<label for="{$ofilter.id}">&nbsp;{$ofilter.label}</label><br/>
  {/foreach}

  <div style="display:block;width=100%;border-top:1px solid #AAAAAA;"></div>

 {$SCOPE}

 <table style="width:100%;border-top:1px solid #B0B0B0;">
  <tr>
   <td>
    <label for="NAME">
     <img src="geticon.php?context=actions&amp;icon=system-search&amp;size=16"/>&nbsp;{t}Name{/t}
    </label>
   </td>
   <td>
    {$NAME}
   </td>
  </tr>
 </table>

 <table width="100%" style="background:#EEEEEE;border-top:1px solid #B0B0B0;">
  <tr>
   <td style="width:100%;text-align:right;">
    {$APPLY}
   </td>
  </tr>
 </table>
</div>
