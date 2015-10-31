<table width="100%">
 <tr>
  <td style="vertical-align:top; width:50%; border-right:1px solid #B0B0B0">
{if $StandAlone}
   <h2>{t}General{/t}</h2>
   <table>
    <tr>
     <td><LABEL for="cn" >{t}Printer name{/t}</LABEL>{$must}</td>
     <td>
{render acl=$cnACL}
      <input type='text' name="cn" id="cn" size=20 maxlength=60 value="{$cn}">
{/render}
     </td>
    </tr>
    <tr>
     <td><LABEL for="description">{t}Description{/t}</LABEL></td>
     <td>
{render acl=$descriptionACL}
      <input type='text' id="description" name="description" size=25 maxlength=80 value="{$description}">
{/render}
     </td>
    </tr>
    <tr>
      <td colspan=2>&nbsp;</td>
    </tr>
    <tr>
     <td><LABEL for="base">{t}Base{/t}</LABEL>{$must}</td>
     <td>
{render acl=$baseACL}
       {$base}
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td>
{/if}
   <h2>{t}Details{/t}</h2>
   <table>
{if !$StandAlone}
      <tr>
  <td><LABEL for="description">{t}Description{/t}</LABEL></td>
  <td>
  {render acl=$descriptionACL}
     <input type='text' id="description" name="description" size=25 maxlength=80 value="{$description}">
  {/render}
   </td>
      </tr>
{/if}
     <tr>
       <td><LABEL for="l">{t}Printer location{/t}</LABEL></td>
       <td>
{render acl=$lACL}
        <input type='text' id="l" name="l" size=30 maxlength=80 value="{$l}">
{/render}
       </td>
     </tr>
     <tr>
       <td><LABEL for="labeledURI">{t}Printer URL{/t}</LABEL>{$must}</td>
       <td>
{render acl=$labeledURIACL}
        <input type='text' id="labeledURI" name="labeledURI" size=30 maxlength=80 value="{$labeledURI}">
{/render}
       </td>
     </tr>
{if $displayServerPath && 0}
    <tr>
     <td>{t}PPD Provider{/t}
     </td>
     <td>
      <input size=30 type='text' value='{$ppdServerPart}' name='ppdServerPart'>
     </td>
    </tr>
{/if}
   </table>
   <table>
    <tr>
     <td>
      <br>
      {t}Driver{/t}: <i>{$driverInfo}</i>&nbsp;
{render acl=$gotoPrinterPPDACL mode=read_active}
       <input type="submit" name="EditDriver" value="{t}Edit{/t}">
{/render}
{render acl=$gotoPrinterPPDACL}
       <input type="submit" name="RemoveDriver" value="{t}Remove{/t}">
{/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>

<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>

<h2><img class="center" alt="" src="geticon.php?context=categories&icon=acl&size=16"> {t}Permissions{/t}</h2>
<table width="100%">
 <tr>
  <td style="border-right:1px solid #B0B0B0; width:50%">
   <table style="width:100%">
    <tr>
     <td>
      {t}Users which are allowed to use this printer{/t}<br>
{render acl=$gotoUserPrinterACL}
      <select size="1" name="UserMember[]" title="{t}Users{/t}" style="width:100%;height:120px;"  multiple>
       {html_options options=$UserMembers values=$UserMemberKeys}
      </select><br>
{/render}
{render acl=$gotoUserPrinterACL}
      <input type="submit" value="{msgPool type=addButton}"  name="AddUser">
{/render}
{render acl=$gotoUserPrinterACL}
      <input type="submit" value="{msgPool type=delButton}" name="DelUser">
{/render}
     </td>
    </tr>
   </table>

  </td>
  <td>
   <table style="width:100%">
    <tr>
     <td>
      {t}Users which are allowed to administrate this printer{/t}<br>
{render acl=$gotoUserPrinterACL}
           <select size="1" name="AdminMember[]" title="{t}Admins{/t}" style="width:100%;height:120px;"  multiple>
                    {html_options options=$AdminMembers values=$AdminMemberKeys}
                   </select><br>
{/render}
{render acl=$gotoUserPrinterACL}
       <input type="submit" value="{msgPool type=addButton}"  name="AddAdminUser">
{/render}
{render acl=$gotoUserPrinterACL}
       <input type="submit" value="{msgPool type=delButton}" name="DelAdmin">
{/render}

     </td>
    </tr>
   </table>

  </td>
 </tr>
</table>

<p class="plugbottom" style="margin-bottom:0px; padding:0px;">&nbsp;</p>
{if $netconfig ne ''}
<table style="width:100%">
<tr>
<td style="vertical-align:top; width:50%; border-right:1px solid #B0B0B0">
{$netconfig}
</td>
<td>
{/if}
<h2>{t}Windows paths{/t}</h2>
<table style="width:100%">
     <tr>
       <td><LABEL for="printerWindowsInfFile">{t}Printer windows inf file{/t}</LABEL></td>
       <td>
{render acl=$printerWindowsInfFileACL}
        <input type='text' id="printerWindowsInfFile" name="printerWindowsInfFile" value="{$printerWindowsInfFile}">
{/render}
       </td>
     </tr>
     <tr>
       <td><LABEL for="printerWindowsDriverDir">{t}Printer driver directory{/t}</LABEL></td>
       <td>
{render acl=$printerWindowsDriverDirACL}
        <input type='text' id="printerWindowsDriverDir" name="printerWindowsDriverDir" value="{$printerWindowsDriverDir}">
{/render}
       </td>
     </tr>
     <tr>
       <td><LABEL for="printerWindowsDriverName">{t}Driver printer name{/t}</LABEL></td>
       <td>
{render acl=$printerWindowsDriverNameACL}
        <input type='text' id="printerWindowsDriverName" name="printerWindowsDriverName" value="{$printerWindowsDriverName}">
{/render}
       </td>
     </tr>
<table>
{if $netconfig ne ''}
</td></tr></table>
{/if}


<!-- Place cursor -->
<script type="text/javascript">

  <!-- // First input field on page
  if(document.mainform.cn)
  focus_field('cn');
  -->
</script>
